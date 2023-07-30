use std::collections::VecDeque;
use std::num::NonZeroUsize;
use std::sync::Arc;

use rand::{Rng};

use crate::geometry::line_intersection;
use crate::room_state::{RoomState, Pos};

trait Solver {

}

struct RoomState2 {
  pub points_signal: Vec<f64>,
}

struct ParamRanges {
  pub points_limits: Vec<(f64, f64)>,
}

struct MonteCarloSolver {
}

struct StateToSolve {
  y_from: usize,
  y_to: usize,
  new_state: Arc<RoomState2>,
  regular: Arc<RoomState>,
}

fn solve_slice(state: StateToSolve) {
  for y in state.y_from..state.y_to {
    for x in 0..state.map_size {
      let pixel = Pos::new(x as f64 / state.map_size as f64 , y as f64 / state.map_size as f64);
      let mut dBm = 1000f64;
      // Рассматриваем каждую точку доступа
      for point in &state.regular.radio_points {
        let mut damped = 0f64;

        // Смотрим сколько стен пересекаются с лучём видимости.
        for wall in &state.regular.walls {
          if line_intersection((pixel, *point), *wall).is_some() {
            damped += wall.damping;
          }
        }
        // Считаем силу сигнала и запоминаем самый сильный
        let dBc = getDBm(&pixel, point, 1.0, count_wall) / 1000.0;
        if dBc < dBm  {
          dBm = dBc;
        }
      }
    }
  }
}

impl MonteCarloSolver {
  pub async fn try_montecarlo() {
    let state = Arc::new(crate::room_state::get_config());

    let limits = ParamRanges {
      points_limits: state.radio_points.iter().map(|p| (p.power_min, p.power_max)).collect(),
    };

    let mut best_match = None;

    let mut rng = rand::thread_rng();

    let mut backlog = VecDeque::new();

    let threads = std::thread::available_parallelism().unwrap_or(NonZeroUsize::MIN).get();

    let (tx, rx) = tokio::sync::mpsc::channel::<StateToSolve>(4*threads);

    tokio::spawn(async move {
      while let Some(new_state) = rx.recv().await {
        solve_slice(new_state);
      }
    });

    loop {
      let next_guess = Arc::new(RoomState2 {
        points_signal: limits.points_limits.iter().map(|(min, max)| rng.gen_range(*min..=*max)).collect(),
      });

      for (y_from, y_to) in chop_ys(map_size, threads) {
        tx.send(StateToSolve {
          y_from,
          y_to,
          new_state: next_guess.clone(),
          regular: state.clone(),
        }).await;
      }
    }
  }
}

pub fn chop_ys(y_max: usize, mut slices: usize) -> impl Iterator<Item = (usize, usize)> {
  slices = slices.min(y_max);
  (0..slices)
    .map(move |i| (
      y_max *  i    / slices,
      y_max * (i+1) / slices
    )
  )
}
