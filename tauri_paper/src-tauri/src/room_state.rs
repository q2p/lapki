use std::sync::Mutex;

use serde::{Serialize, Deserialize};

#[derive(Serialize, Deserialize, Debug, Copy, Clone)]
pub struct Pos {
  pub x: f64,
  pub y: f64,
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
  pub id: usize,
  pub pos: Pos,
  pub power: f64,
  pub power_min: f64,
  pub power_max: f64,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct RadioZone {
  pub desired_point_id: usize,
  pub points: Vec<Pos>
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
pub fn get_config() -> RoomState {
  STATE.lock().unwrap().clone()
}