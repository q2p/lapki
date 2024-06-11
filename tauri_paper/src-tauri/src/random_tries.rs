use std::num::NonZeroUsize;
use std::ops::Deref;
use std::sync::atomic::Ordering;
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};

use rand::{Rng, SeedableRng};
use serde::Serialize;

use crate::heatmap::{self, bounding_box, do_calc_sinr_dbm, is_inside, pix_to_meter, BSP, clamp_rad};
use crate::room_state::{get_state, Pos, Px, RoomLayout};
use crate::RUNNING;

struct Ticker {
    next: Instant,
    lt: u64,
    rem: u64,
}
impl Ticker {
  const DIST: Duration = Duration::from_millis(1000);
  pub fn new() -> Ticker {
    Ticker {
      next: Instant::now(),
      lt: 0,
      rem: 0,
    }
  }
  pub fn tick(&mut self) -> bool {
    if self.rem != self.lt {
      self.rem += 1;
      return false;
    }
    let now = Instant::now();
    let prev = self.next - Ticker::DIST;
    let passed = (now - prev).as_secs_f64() / Ticker::DIST.as_secs_f64();
    self.lt = ((self.lt as f64 / passed).ceil() as u64).max(1);
    if now < self.next {
      return false;
    }
    self.rem = 0;
    while self.next < now {
      self.next += Ticker::DIST;
    }
    return true;
  }
}

#[derive(Clone)]
pub struct SignalStrength {
  pub points_signal_mws: Vec<f64>,
  pub points_directions_rad: Vec<f64>,
}

#[derive(Debug)]
struct ParamRanges {
  /// mw min, max
  pub points_limits_mws: Vec<(f64, f64)>,
}

fn solve_median(bb: &BoundingBoxes, signal_strength: &SignalStrength, layout: &RoomLayout, bsp: &BSP) -> f64 {
  let mut sinrs = Vec::with_capacity(bb.res.0 * bb.res.1);
  for y in 0..bb.res.1 {
    for x in 0..bb.res.0 {
      let pix = pix_to_meter(&bb, Px::new(x as isize, y as isize));
      if let Some(z) = layout.radio_zones.iter().find(|zone| is_inside(pix, zone)) {
        let sinr = do_calc_sinr_dbm(&layout, &bsp, &signal_strength, z, pix);
        sinrs.push(sinr);
      }
    }
  }
  if sinrs.is_empty() {
    return 0.0;
  }
  sinrs.sort_by(f64::total_cmp);
  let median_idx = (sinrs.len() as f64 * 0.5) as usize;
  let min_idx = (sinrs.len() as f64 * 0.1) as usize;
  let median = sinrs[median_idx];
  let min = sinrs[min_idx];
  let lerp = (min - MIN_SINR) / (MIN_SINR * MIN_SINR_LERP);
  if lerp > 1.0 {
    return median;
  }
  if lerp < 0.0 {
    return -(lerp * lerp);
  }
  return median * lerp;
}

#[derive(Debug, Default, Clone, Serialize)]
pub struct BoundingBoxes {
  pub min: Pos,
  pub max: Pos,
  pub wh: Pos,
  pub res: (usize, usize),
}

impl BoundingBoxes {
  pub fn new(points: impl Iterator<Item = impl Deref<Target = Pos>>, padding: f64, pixels: u32) -> BoundingBoxes {
    let (mut min, mut max) = bounding_box(points);
    min.x -= padding;
    min.y -= padding;
    max.x += padding;
    max.y += padding;
    let wh = max - min;

    let res_x = ((pixels as f64 / (wh.y / wh.x)).sqrt() as usize).max(1);
    let res_y = ((pixels as f64 / (wh.x / wh.y)).sqrt() as usize).max(1);
    return BoundingBoxes {
      min,
      max,
      wh,
      res: (res_x, res_y),
    };
  }

  const ZERO: Self = BoundingBoxes {
    min: Pos::ZERO,
    max: Pos::ZERO,
    res: (0, 0),
    wh: Pos::ZERO
  };
}

pub static RENDERING_BB: Mutex<BoundingBoxes> = Mutex::new(BoundingBoxes::ZERO);

pub async fn do_montecarlo() {
  let threads = std::thread::available_parallelism().unwrap_or(NonZeroUsize::MIN).get();

  let (best_tx, mut best_rx) = tokio::sync::watch::channel::<Option<Arc<Exch>>>(None);

  {
    tokio::spawn(async move {
      while let Ok(()) = best_rx.changed().await {
        let ns = match best_rx.borrow().as_ref() {
          Some(ns) => ns.clone(),
          None => continue,
        };
        {
          *RENDERING_BB.lock().unwrap() = BoundingBoxes::clone(&ns.render);
        }
        heatmap::next_image(ns.render.clone(), ns.layout.clone(), ns.bsp.clone(), ns.signal_strength.clone()).await;
      }
    });
  }

  let best_tx = Arc::new(best_tx);

  let global_max_sinr = Arc::new(Mutex::new(f64::NEG_INFINITY));

  for i in 0..threads {
    let best_tx = Arc::clone(&best_tx);
    let global_max_sinr = Arc::clone(&global_max_sinr);
    tokio::spawn(async move {
      let mut rng = rand::rngs::StdRng::from_entropy();

      let mut receiver = get_state().subscribe();

      let mut layout = Arc::new(RoomLayout {
        walls: vec![],
        radio_points: vec![],
        radio_zones: vec![],
      });
      let mut bsp = BSP::new(&layout);
      let mut measure = Arc::new(BoundingBoxes::default());
      let mut render = Arc::new(BoundingBoxes::default());
      let mut limits = Arc::new(ParamRanges { points_limits_mws: vec![] });

      let mut iterations = 0;

      let mut local_max_sinr = f64::NEG_INFINITY;

      let mut ticker = Ticker::new();

      loop {
        if iterations & 0b11 == 0 {
          while !RUNNING.load(Ordering::SeqCst) {
            tokio::time::sleep(Duration::from_millis(100)).await;
          }
          if i == 0 && ticker.tick() {
            println!("Iterations: {}", iterations * threads);
          }

          if let Ok(true) = receiver.has_changed() {
            layout = Arc::new(receiver.borrow_and_update().clone());
            measure = Arc::new(BoundingBoxes::new(layout.walls.iter().flat_map(|w| [&w.a, &w.b]), 0.5, 128*128));
            render = Arc::new(BoundingBoxes::new(layout.walls.iter().flat_map(|w| [&w.a, &w.b]), 2.0, 2048*2048));
            bsp = BSP::new(&layout);
            limits = Arc::new(ParamRanges {
              points_limits_mws: layout.radio_points.iter().map(|p| (p.power_min_mw, p.power_max_mw)).collect(),
            });
          }
          tokio::task::yield_now().await;
        }
        iterations += 1;
        let next_guess = SignalStrength {
          points_signal_mws: limits.points_limits_mws.iter().map(|(min, max)| rng.gen_range(*min..=*max)).collect(),
          points_directions_rad: limits.points_limits_mws.iter().map(|_| rng.gen_range(-std::f64::consts::PI..=std::f64::consts::PI)).collect(),
        };

        let sinr = solve_median(&measure, &next_guess, &layout, &bsp);
        if sinr > local_max_sinr {
          local_max_sinr = sinr;
          let mut gms = global_max_sinr.lock().unwrap();
          if local_max_sinr > *gms {
            *gms = local_max_sinr;
            drop(gms);
            best_tx.send(Some(Arc::new(Exch {
              layout: layout.clone(),
              bsp: bsp.clone(),
              signal_strength: Arc::new(next_guess),
              measure: measure.clone(),
              render: render.clone(),
              max_value: sinr,
              limits: limits.clone(),
            }))).unwrap();
          }
        }
      }
    });
  }
}

struct Exch {
  max_value: f64,

  layout: Arc<RoomLayout>,
  bsp: Arc<BSP>,
  signal_strength: Arc<SignalStrength>,
  limits: Arc<ParamRanges>,
  measure: Arc<BoundingBoxes>,
  render: Arc<BoundingBoxes>,
}

const MOMENTUM_FACTOR: f64 = 0.95;
const LEARNING_RATE: f64 = 0.5;
const MOMENTUM_TIMEOUT: usize = 5;
const MOMENTUM_THRESHOLD: f64 = 0.1;

pub async fn do_nesterov(mut best_rx: tokio::sync::watch::Receiver<Option<Arc<Exch>>>) {
  let threads = std::thread::available_parallelism().unwrap_or(NonZeroUsize::MIN).get();

  {
    tokio::spawn(async move {
      while let Ok(()) = best_rx.changed().await {
        let exch = match best_rx.borrow().as_ref() {
          Some(v) => Arc::clone(v),
          None => continue,
        };
        println!("Nesterov new");

        let mut iterations = 0;

        let mut local_max_sinr = f64::NEG_INFINITY;

        let mut ticker = Ticker::new();

        let mut position = SignalStrength::clone(&exch.signal_strength);
        let mut velocity = SignalStrength {
          points_signal_mws: vec![0.0; position.points_signal_mws.len()],
          points_directions_rad: vec![0.0; position.points_directions_rad.len()],
        };

        let mut iters_since_small = 0;
        while iters_since_small < MOMENTUM_TIMEOUT {
          if iterations & 0b11 == 0 {
            while !RUNNING.load(Ordering::SeqCst) {
              tokio::time::sleep(Duration::from_millis(100)).await;
            }
            if ticker.tick() {
              println!("Nesterov Iterations: {}", iterations * threads);
            }
            tokio::task::yield_now().await;
          }
          iterations += 1;

          let mut look_ahead = position.clone();
          for i in 0..exch.limits.points_limits_mws.len() {
            look_ahead.points_signal_mws[i] += MOMENTUM_FACTOR * velocity.points_signal_mws[i];
          }

          let mut handles = Vec::with_capacity(2 * exch.limits.points_limits_mws.len());
          for i in 0..exch.limits.points_limits_mws.len() {
            for dir in [-0.01, 0.01] {
              let mut ss = look_ahead.clone();
              let exch = exch.clone();
              handles.push(tokio::task::spawn(async move {
                let ss_ref = &mut ss.points_signal_mws[i];
                let limit = &exch.limits.points_limits_mws[i];
                *ss_ref = f64::clamp(*ss_ref + dir, limit.0, limit.1);
                let value = solve_median(&exch.measure, &ss, &exch.layout, &exch.bsp);
                // TODO: try_send_best_sinr(optimized_value, &ss);
                return (ss, value)
              }))
            }
            for dir in [-0.01, 0.01] {
              let mut ss = look_ahead.clone();
              let exch = exch.clone();
              handles.push(tokio::task::spawn(async move {
                let ss_ref = &mut ss.points_directions_rad[i];
                *ss_ref = *ss_ref + dir;
                let value = solve_median(&exch.measure, &ss, &exch.layout, &exch.bsp);
                // TODO: try_send_best_sinr(optimized_value, &ss);
                return (ss, value)
              }))
            }
          }

          let mut grad = SignalStrength {
            points_signal_mws: vec![0.0; position.points_signal_mws.len()],
            points_directions_rad: vec![0.0; position.points_directions_rad.len()],
          };
          for handle in handles.into_iter() {
            let (ss, value) = handle.await.unwrap();
            assert_eq!(ss.points_signal_mws.len(), grad.points_signal_mws.capacity());
            for i in 0..grad.points_signal_mws.capacity() {
              grad.points_signal_mws[i] += value / (ss.points_signal_mws[i] - exch.signal_strength.points_signal_mws[i]);
              grad.points_directions_rad[i] += value / (ss.points_directions_rad[i] - exch.signal_strength.points_directions_rad[i]);
            }
          }

          let mut small_velocity = true;
          for i in 0..exch.limits.points_limits_mws.len() {
            {
              let vel = &mut velocity.points_signal_mws[i];
              let grad = grad.points_signal_mws[i];
              let pos = &mut position.points_signal_mws[i];
              *vel += *vel * MOMENTUM_FACTOR + grad * LEARNING_RATE;
              *pos += *vel;
              small_velocity &= *vel < MOMENTUM_THRESHOLD
            }
            {
              let vel = &mut velocity.points_directions_rad[i];
              let grad = grad.points_directions_rad[i];
              let pos = &mut position.points_directions_rad[i];
              *vel += *vel * MOMENTUM_FACTOR + grad * LEARNING_RATE;
              *pos = clamp_rad(*pos + *vel);
              small_velocity &= *vel < MOMENTUM_THRESHOLD
            }
          }
          if small_velocity {
            iters_since_small += 1;
          } else {
            iters_since_small = 0;
          }
        }

        println!("Quit Nesterov after {iterations} iterations.");
      }
    });
  }
}

const MIN_SINR_LERP: f64 = 0.1;
const MIN_SINR: f64 = 0.9;

pub fn chop_ys(y_max: usize, mut slices: usize) -> impl Iterator<Item = (usize, usize, usize)> {
  slices = slices.min(y_max);
  (0..slices)
    .map(move |i| (
      y_max *  i    / slices,
      y_max * (i+1) / slices,
      slices
    )
  )
}
