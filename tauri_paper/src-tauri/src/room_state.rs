use std::sync::Arc;
use std::sync::{atomic::Ordering, Mutex};
use std::str::FromStr;

use serde::{Serialize, Deserialize};
use tokio::sync::watch::Sender;

use crate::random_tries::{BoundingBoxes, RENDERING_BB};
use crate::RUNNING;

#[derive(Serialize, Deserialize, Default, Debug, Copy, Clone)]
pub struct Pos {
  pub x: f64,
  pub y: f64,
}

impl core::ops::Sub for Pos {
  type Output = Pos;
  fn sub(self, rhs: Pos) -> Pos {
    Pos {
      x: self.x - rhs.x,
      y: self.y - rhs.y,
    }
  }
}

impl core::ops::Add for Pos {
  type Output = Pos;
  fn add(self, rhs: Pos) -> Pos {
    Pos {
      x: self.x + rhs.x,
      y: self.y + rhs.y,
    }
  }
}

impl core::ops::Mul<f64> for Pos {
  type Output = Pos;
  fn mul(self, rhs: f64) -> Pos {
    Pos {
      x: self.x * rhs,
      y: self.y * rhs,
    }
  }
}

#[derive(Serialize, Deserialize, Debug, Copy, Clone)]
pub struct Px {
  pub x: isize,
  pub y: isize,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct RoomLayout {
  pub walls: Vec<Wall>,
  pub radio_points: Vec<RadioPoint>,
  pub radio_zones: Vec<RadioZone>,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct Wall {
  pub a: Pos,
  pub b: Pos,
  pub damping: f64,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct RadioPoint {
  pub pos: Pos,
  pub power: f64,
  pub power_min_mw: f64,
  pub power_max_mw: f64,
  pub ang_min: f64,
  pub ang_max: f64,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct RadioZone {
  pub desired_point_id: usize,
  /// По часовой стрелке.
  pub points: Vec<Pos>,
  pub r: u8,
  pub g: u8,
  pub b: u8
}

impl Pos {
  pub const fn new(x: f64, y: f64) -> Pos {
    Pos { x, y }
  }

  pub const ZERO: Pos = Pos::new(0.0, 0.0);

  pub fn dot(v1: &Pos, v2: &Pos) -> f64 {
    v1.x * v2.x + v1.y * v2.y
  }

  pub fn cross(v1: &Pos, v2: &Pos) -> f64 {
    v1.x * v2.y - v1.y * v2.x
  }
}

impl Px {
  pub fn new(x: isize, y: isize) -> Px {
    Px { x, y }
  }
}

static STATE: Mutex<Option<Arc<Sender<RoomLayout>>>> = Mutex::new(None);
pub fn get_state() -> Arc<Sender<RoomLayout>> {
  return Arc::clone(&STATE.lock().unwrap().get_or_insert_with(|| {
    Arc::new(Sender::new(RoomLayout {
      walls: Vec::new(),
      radio_points: Vec::new(),
      radio_zones: Vec::new(),
    }))
  }));
}

#[derive(Clone)]
pub struct Range {
  pub min: f64,
  pub max: f64,
  pub pow: f64,
}
pub static RANGE: Mutex<Range> = Mutex::new(Range {
  min: 0.31,
  max: 0.368,
  pow: 16.0,
});

pub fn write_config(config: RoomLayout, path: &str) {
  let json = serde_json::to_string(&config).unwrap();
  {
    let _ = get_state().send_replace(config);
  }
  // std::fs::write("map.json", json).unwrap();
  std::fs::write(path, json).unwrap();
}

pub fn load_config(path: &str) {
  if let Ok(file) = std::fs::read_to_string(path) {
    let _ = get_state().send_replace(serde_json::from_str(&file).unwrap());
  }
}

pub fn get_config2() -> RoomLayout {
  let a: RoomLayout = Sender::borrow(&get_state()).clone();
  return a;
}

#[tauri::command]
pub fn cmd_do(message: String) {
  let mut args:Vec<&str> = message.split(" ").collect();
  match args.remove(0) {
    "max" => {
      RANGE.lock().unwrap().max = f64::from_str(&args[1]).unwrap();
    }
    "min" => {
      RANGE.lock().unwrap().min = f64::from_str(&args[1]).unwrap();
    }
    "pow" => {
      RANGE.lock().unwrap().pow = f64::from_str(&args[1]).unwrap();
    }
    _ => {}
  }
}

#[tauri::command]
pub fn should_play(should_play: bool) {
  RUNNING.store(should_play, Ordering::SeqCst);
}

#[tauri::command]
pub fn get_config(path: &str) -> RoomLayout {
  load_config(path);
  return Sender::borrow(&get_state()).clone();
}

#[tauri::command]
pub fn save_config(config: RoomLayout, path: &str) {
  write_config(config, path)
}

#[tauri::command]
pub fn get_bb() -> BoundingBoxes {
  RENDERING_BB.lock().unwrap().clone()
}


