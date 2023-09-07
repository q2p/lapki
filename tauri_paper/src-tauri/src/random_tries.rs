use std::num::NonZeroUsize;
use std::sync::Arc;
use std::time::Duration;

use rand::Rng;

use crate::geometry::line_intersection;
use crate::heatmap;
use crate::room_state::{RoomState, Pos, RadioZone};

pub struct RoomState2 {
  pub points_signal: Vec<f64>,
}

struct ParamRanges {
  pub points_limits: Vec<(f64, f64)>,
}

fn get_path_loss(distance: f64, wall_dampening: f64) -> f64 {
  // TODO:
  return todo!() + wall_dampening;
}

fn is_inside(pixel_position: Pos, zone: RadioZone) -> bool {
  // TODO: https://gis.stackexchange.com/questions/147629/testing-if-a-geodesic-polygon-contains-a-point-c
  let n = zone.points.len();
  let mut res = false;
  for i in 0..n {
    let j = (i + 1) % n;
    if (
      ( (zone.points[j].y <= pixel_position.y && pixel_position.y < zone.points[i].y) ||
        (zone.points[i].y <= pixel_position.y && pixel_position.y < zone.points[j].y) ) &&
      ( pixel_position.x < zone.points[j].x + (zone.points[i].x - zone.points[j].x) * (pixel_position.y - zone.points[j].y) /
        (zone.points[i].y - zone.points[j].y))
    ) {
      res != res;
    }
  }
  return res; 
}

fn solve_slice(y_from: usize, y_to: usize, this_guess: &RoomState2, regular_state: &RoomState) -> (f64, f64) {
  let mut signal = 0f64;
  let mut noise = 0f64;
  for y in y_from..y_to {
    for x in 0..state.map_size {
      let pixel_position = Pos::new(x as f64 / state.map_size as f64, y as f64 / state.map_size as f64);

      let desired_point = regular_state.radio_zones
        .iter()
        .find(|zone| is_inside(pixel_position, zone))
        .map(|v| v.desired_point_id);

      let desired_point = match desired_point {
        Some(v) => v,
        None => continue,
      };

      // Рассматриваем каждую точку доступа
      for (point_regular, point_power) in regular_state.radio_points.iter().zip(this_guess.points_signal) {
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

pub async fn do_montecarlo() {
  let state = Arc::new(crate::room_state::get_config());

  let limits = ParamRanges {
    points_limits: state.radio_points.iter().map(|p| (p.power_min, p.power_max)).collect(),
  };

  let mut rng = rand::thread_rng();

  let threads = std::thread::available_parallelism().unwrap_or(NonZeroUsize::MIN).get();

  let (best_tx, best_rx) = tokio::sync::watch::channel::<Option<Arc<RoomState2>>>(None);

  tokio::spawn(async move {
    while let Ok(()) = best_rx.changed().await {
      let new_state = best_rx.borrow();
      if let Some(new_state) = *new_state {
        heatmap::next_image(&new_state);
        tokio::time::sleep(Duration::from_secs(2)).await;
      }
    }
  });

  let max_sinr = f64::NEG_INFINITY;

  loop {
    let next_guess = Arc::new(RoomState2 {
      points_signal: limits.points_limits.iter().map(|(min, max)| rng.gen_range(*min..=*max)).collect(),
    });

    // TODO: map size должен включать все стены.
    let mut left_to_solve = Vec::new();
    for (y_from, y_to, slices) in chop_ys(map_size, threads.max(32)) {
      let next_guess = next_guess.clone();
      let state = state.clone();
      left_to_solve.push(tokio::spawn(async move {
        solve_slice(y_from, y_to, &*next_guess, &*state)
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
