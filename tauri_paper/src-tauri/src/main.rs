// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use std::path::Path;
use serde::{Deserialize, Serialize};
use std::fs::File;
use std::io::{BufReader, BufWriter, Write};

// Learn more about Tauri commands at https://tauri.app/v1/guides/features/command
#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Rust!", name)
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(rename_all = "camelCase")]
struct Config {
  walls: Vec<Wall>,
  radio_points: Vec<RadioPoint>,
  radio_zones: Vec<RadioZone>,
}

#[derive(Serialize, Deserialize, Debug)]
struct Wall {
  a: Point2d,
  b: Point2d,
  damping: i32,
}

#[derive(Serialize, Deserialize, Debug)]
struct RadioPoint {
  pos: Point2d,
  power: i32,
}

#[derive(Serialize, Deserialize, Debug)]
struct RadioZone {
  points: Vec<Point2d>
}

#[derive(Serialize, Deserialize, Debug)]
struct Point2d {
  x: i32,
  y: i32,
}

fn read_config<P: AsRef<Path>>(path: P) -> Config {
  let file = File::open(path).unwrap();
  let reader = BufReader::new(file);

  let c: Config = serde_json::from_reader(reader).unwrap();
  c
}

fn write_config<P: AsRef<Path>>(config: Config, path: P){
  let file = File::create(path).unwrap();
  let mut writer = BufWriter::new(file);
  let json = serde_json::to_string(&config).unwrap();
  writer.write_all(json.as_bytes()).expect("unable to write");
}

#[tauri::command]
fn get_config() -> Config {
  read_config("config.json")
}

fn main() {
  // for a in std::fs::read_dir(".").unwrap() {
  //   println!("{}", a.unwrap().path().to_str().unwrap())
  // }
  // let mut config = read_config("config.json");
  // config.walls[0].a.x = 6969;
  // write_config(config, "config.json");

  tauri::Builder::default()
    .invoke_handler(tauri::generate_handler![get_config])
    .run(tauri::generate_context!())
    .expect("error while running tauri application");
}
