// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod random_tries;
mod heatmap;
mod room_state;
mod geometry;

fn main() {
  room_state::load_config();
  tauri::Builder::default()
    // .on_page_load(|window, _payload| {
    //   let payload = BootPayload { drives: scan_drive() };
    //   window
    //     .emit("boot", Some(payload))
    //     .expect("failed to emit event");
    // })
    .setup(|app| {
      tauri::async_runtime::spawn(async move { random_tries::do_montecarlo().await });
      Ok(())
    })
    .invoke_handler(tauri::generate_handler![room_state::get_config, heatmap::get_active_best])
    .run(tauri::generate_context!())
    .expect("error while running tauri application");
}