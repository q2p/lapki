use std::sync::Mutex;
use std::str::FromStr;

use serde::{Serialize, Deserialize};

#[derive(Serialize, Deserialize, Debug, Copy, Clone)]
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
pub struct RoomState {
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
  pub power_max_mw: f64,
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
  pub fn new(x: f64, y: f64) -> Pos {
    Pos { x, y }
  }
}

impl Px {
  pub fn new(x: isize, y: isize) -> Px {
    Px { x, y }
  }
}

static STATE: Mutex<RoomState> = Mutex::new(RoomState {
  walls: Vec::new(),
  radio_points: Vec::new(),
  radio_zones: Vec::new(),
});

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

pub fn write_config(config: RoomState) {
  let json = serde_json::to_string(&config).unwrap();
  {
    *STATE.lock().unwrap() = config;
  }
  std::fs::write("map.json", json).unwrap();
}

pub fn load_config() {
  let file = std::fs::read_to_string("map.json").unwrap();
  *STATE.lock().unwrap() = serde_json::from_str(&file).unwrap();
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
pub fn get_config() -> RoomState {
  STATE.lock().unwrap().clone()
}
