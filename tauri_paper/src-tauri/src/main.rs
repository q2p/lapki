// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use std::fs::OpenOptions;
use std::io::BufWriter;
use std::path::Path;
use std::time::Duration;

mod thread_pool;
mod heatmap;

// Learn more about Tauri commands at https://tauri.app/v1/guides/features/command
#[tauri::command]
fn greet(name: &str) -> String {
  format!("Hello, {}! You've been greeted from Rust!", name)
}

fn main() {
  tauri::Builder::default()
    // .manage(DriveEntries(scan_drive().into()))
    // .manage(ActiveDrive(0.into()))
    // .manage(ActivePath(Default::default()))
    // .manage(DirEntries(Default::default()))
    // .manage(SubDirectoriesCount(Default::default()))
    .invoke_handler(tauri::generate_handler![
      greet,
      // change_drive,
      // scan_dir,
      // change_dir,
      // count_sub_dir,
    ])
    // .on_page_load(|window, _payload| {
    //   let payload = BootPayload { drives: scan_drive() };
    //   window
    //     .emit("boot", Some(payload))
    //     .expect("failed to emit event");
    // })
    .setup(|app| {
      tauri::async_runtime::spawn(async move { heatmap::next_image().await });
      Ok(())
    })
    .run(tauri::generate_context!())
    .expect("error while running tauri application");
  println!("finished");
}
