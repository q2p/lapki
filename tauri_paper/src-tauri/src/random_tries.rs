use std::num::NonZeroUsize;
use std::ops::Deref;
use std::sync::Arc;
use std::time::Duration;

use rand::{Rng, SeedableRng};

use crate::geometry::line_intersection;
use crate::heatmap::{self, is_inside, bounding_box, pix_to_meter};
use crate::room_state::{RoomState, Pos, Px};

pub struct RoomState2 {
  pub points_signal: Vec<f64>,
}

struct ParamRanges {
  pub points_limits: Vec<(f64, f64)>,
}

fn get_path_loss(distance: f64, wall_dampening: f64) -> f64 {
  // TODO:
  return wall_dampening + 0.0;
}

fn solve_slice(bb: &BoundingBoxes, y_from: usize, y_to: usize, this_guess: &RoomState2, regular_state: &RoomState) -> (f64, f64) {
  let mut signal = 0f64;
  let mut noise = 0f64;
  for y in y_from..y_to {
    for x in 0..bb.res.0 {
      let pixel_position = pix_to_meter(&bb, Px::new(x as isize, y as isize));

      let desired_point = regular_state.radio_zones
        .iter()
        .find(|zone| is_inside(pixel_position, zone))
        .map(|v| v.desired_point_id);

      let desired_point = match desired_point {
        Some(v) => v,
        None => continue,
      };

      // Рассматриваем каждую точку доступа
      for (point_regular, point_power) in regular_state.radio_points.iter().zip(this_guess.points_signal.iter()) {
        let distance = {
          let dx = pixel_position.x - point_regular.pos.x;
          let dy = pixel_position.y - point_regular.pos.y;
          (dx*dx + dy*dy).sqrt()
        };

        let mut wall_dampening = 0f64;
        // Смотрим сколько стен пересекаются с лучём видимости.
        for wall in &regular_state.walls {
          if line_intersection((pixel_position, point_regular.pos), (wall.a, wall.b)).is_some() {
            wall_dampening += wall.damping;
          }
        }
        // Считаем силу сигнала и запоминаем самый сильный
        let final_signal_power = point_power - get_path_loss(distance, wall_dampening);

        if point_regular.id == desired_point {
          signal += final_signal_power;
        } else {
          noise += final_signal_power;
        }
      }
    }
  }
  return (signal, noise);
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

  let limits = ParamRanges {
    points_limits: state.radio_points.iter().map(|p| (p.power_min, p.power_max)).collect(),
  };

  let mut rng = rand::rngs::StdRng::from_entropy();

  let threads = std::thread::available_parallelism().unwrap_or(NonZeroUsize::MIN).get();

  let (best_tx, mut best_rx) = tokio::sync::watch::channel::<Option<Arc<RoomState2>>>(None);

  let measure = Arc::new(BoundingBoxes::new(state.walls.iter().flat_map(|w| [&w.a, &w.b]), 2.0, 64*64));
  let render = Arc::new(BoundingBoxes::new(state.walls.iter().flat_map(|w| [&w.a, &w.b]), 2.0, 512*512));

  println!("m{:?}", measure);
  println!("r{:?}\n\n", render);

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

  let mut max_sinr = f64::NEG_INFINITY;

  loop {
    let next_guess = Arc::new(RoomState2 {
      points_signal: limits.points_limits.iter().map(|(min, max)| rng.gen_range(*min..=*max)).collect(),
    });

    // TODO: map size должен включать все стены.
    let mut left_to_solve = Vec::new();
    for (y_from, y_to, slices) in chop_ys(measure.res.1, threads.max(32)) {
      let next_guess = next_guess.clone();
      let state = state.clone();
      let measure = measure.clone();
      left_to_solve.push(tokio::spawn(async move {
        solve_slice(&*measure, y_from, y_to, &*next_guess, &*state)
      }));
    }
    let mut signal = 0f64;
    let mut noise = 0f64;
    for i in left_to_solve {
      let (s, n) = i.await.unwrap();
      signal += s;
      noise += n;
    }
    let sinr = signal / (signal + noise);
    if sinr > max_sinr {
      max_sinr = sinr;
      best_tx.send(Some(next_guess)).unwrap();
    }
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
