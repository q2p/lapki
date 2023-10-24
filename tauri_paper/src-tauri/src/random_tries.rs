use std::num::NonZeroUsize;
use std::ops::Deref;
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};

use rand::{Rng, SeedableRng};

use crate::heatmap::{self, is_inside, bounding_box, pix_to_meter, mw_after_walls, STATIC_NOISE_DBM, dbm_to_mw, mw_to_dbm};
use crate::room_state::{RoomState, Pos, Px};

pub struct RoomState2 {
  /// dBm
  pub points_signal_dbm: Vec<f64>,
}

struct ParamRanges {
  /// mw min, max
  pub points_limits_mws: Vec<(f64, f64)>,
}

fn solve_slice(bb: &BoundingBoxes, this_guess: &RoomState2, regular_state: &RoomState) -> (f64, f64) {
  let mut signal_t = 0f64;
  let mut noise_t = 0f64;
  let mut min_sinr = f64::MAX;

  for y in 0..bb.res.1 {
    for x in 0..bb.res.0 {
      let pix = pix_to_meter(&bb, Px::new(x as isize, y as isize));

      let desired_point_id = regular_state.radio_zones
        .iter()
        .find(|zone| is_inside(pix, zone))
        .map(|v| v.desired_point_id);

      let desired_point_id = match desired_point_id {
        Some(v) => v,
        None => continue,
      };

      // Рассматриваем каждую точку доступа
      let mwts: Vec<f64> = regular_state.radio_points
        .iter()
        .zip(this_guess.points_signal_dbm.iter())
        .map(|(point, power_dbm)| mw_after_walls(&regular_state, pix, point, *power_dbm))
        .collect();

      let signal_mw = mwts[desired_point_id];
      let all_signals_mw = mwts.iter().sum::<f64>();
      let interference_mw = all_signals_mw - signal_mw;
      let ni_mw = interference_mw + dbm_to_mw(STATIC_NOISE_DBM);
      let sinr = mw_to_dbm(signal_mw / ni_mw);

      if sinr < min_sinr {
        min_sinr = sinr;
      }

      signal_t += signal_mw;
      noise_t += ni_mw;
    }
  }
  return (signal_t / noise_t, min_sinr);
}

#[derive(Debug)]
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
}

pub async fn do_montecarlo() {
  let state = Arc::new(crate::room_state::get_config());

  let limits = Arc::new(ParamRanges {
    points_limits_mws: state.radio_points.iter().map(|p| (p.power_min_mw, p.power_max_mw)).collect(),
  });

  let threads = std::thread::available_parallelism().unwrap_or(NonZeroUsize::MIN).get();

  let (best_tx, mut best_rx) = tokio::sync::watch::channel::<Option<Arc<RoomState2>>>(None);

  let measure = Arc::new(BoundingBoxes::new(state.walls.iter().flat_map(|w| [&w.a, &w.b]), 2.0, 512*512));
  let render = Arc::new(BoundingBoxes::new(state.walls.iter().flat_map(|w| [&w.a, &w.b]), 2.0, 2048*2048));
  // let measure = Arc::new(BoundingBoxes::new(state.walls.iter().flat_map(|w| [&w.a, &w.b]), 2.0, 32*32));
  // let render = Arc::new(BoundingBoxes::new(state.walls.iter().flat_map(|w| [&w.a, &w.b]), 2.0, 32*32));

  println!("measure {:?}\nrender {:?}\n\n", measure, render);

  {
    let state = state.clone();
    tokio::spawn(async move {
      println!("Entered rx");
      while let Ok(()) = best_rx.changed().await {
        println!("iter_rx");
        let new_state = best_rx.borrow().as_ref().map(|v| v.clone());
        if let Some(ns) = new_state {
          heatmap::next_image(render.clone(), state.clone(), ns.clone()).await;
          tokio::time::sleep(Duration::from_secs(2)).await;
        }
      }
      println!("left rx");
    });
  }

  let best_tx = Arc::new(best_tx);

  let global_max_sinr = Arc::new(Mutex::new(f64::NEG_INFINITY));

  for i in 0..threads {
    let measure = Arc::clone(&measure);
    let state = Arc::clone(&state);
    let limits = Arc::clone(&limits);
    let best_tx = Arc::clone(&best_tx);
    let global_max_sinr = Arc::clone(&global_max_sinr);
    tokio::spawn(async move {
      let mut rng = rand::rngs::StdRng::from_entropy();

      let mut iterations = 0;
      let mut next_check = 0;
      let mut last_print = Instant::now();

      let mut local_max_sinr = f64::NEG_INFINITY;

      let mut super_uper_min = f64::NEG_INFINITY;

      'monte_carlo: loop {
        if iterations & 0b11 == 0 {
          if i == 0 && iterations > next_check {
            next_check += 100;
            let now = Instant::now();
            if now.duration_since(last_print).as_secs() > 0 {
              last_print = last_print + Duration::from_secs(1);
              println!("Iterations: {}", iterations * threads);
            }
          }
          tokio::task::yield_now().await;
        }
        iterations += 1;
        let next_guess = RoomState2 {
          points_signal_dbm: limits.points_limits_mws.iter().map(|(min, max)| mw_to_dbm(rng.gen_range(*min..=*max))).collect(),
          // points_signal_dbm: limits.points_limits_dbm.iter().map(|_| mw_to_dbm(190f64)).collect(),
        };

        let (sinr_avg, min_sinr_dbm) = solve_slice(&*measure, &next_guess, &state);
        // if min_sinr_dbm < 400.0 && min_sinr_dbm > super_uper_min {
        //   super_uper_min = min_sinr_dbm;
        //   println!("min_sinr: {min_sinr_dbm}dbm");
        // }
        if !f64::is_finite(min_sinr_dbm) || min_sinr_dbm < -4.0 {
          continue 'monte_carlo;
        }
        if f64::is_finite(sinr_avg) && sinr_avg > local_max_sinr {
          local_max_sinr = sinr_avg;
          let mut gms = global_max_sinr.lock().unwrap();
          if local_max_sinr > *gms {
            *gms = local_max_sinr;
            drop(gms);
            println!("min_sinr_sum: {}mw", sinr_avg);
            best_tx.send(Some(Arc::new(next_guess))).unwrap();
          }
        }
      }
    });
  }
}

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