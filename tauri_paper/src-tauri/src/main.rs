// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use std::fs::File;
use std::path::Path;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Mutex;
use std::time::Duration;

use random_tries::do_montecarlo;
use serde::{Deserialize, Serialize};
use tauri::{CustomMenuItem, Manager, Menu, MenuEntry, MenuItem, Submenu};

mod geometry;
mod heatmap;
mod random_tries;
mod room_state;

static RUNNING: AtomicBool = AtomicBool::new(false);
static PATH: &str = "5g_planner_config.json";
static APPCONFIG: Mutex<AppConfig> = Mutex::new(AppConfig {
    latest_config: Option::None,
});

#[derive(Clone, Serialize)]
struct NotificationPayload {
    message: String,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
struct AppConfig {
    latest_config: Option<String>,
}

fn load_app_config() {
    if !Path::new(PATH).exists() {
        File::create(PATH).unwrap();
        std::fs::write(PATH, r#"{"latest_config": null}"#).unwrap();
    }
    let file = std::fs::read_to_string(PATH).unwrap();
    *APPCONFIG.lock().unwrap() = serde_json::from_str(&file).unwrap();
}

fn _write_app_config(app_config: AppConfig) {
    let json = serde_json::to_string(&app_config).unwrap();
    {
        *APPCONFIG.lock().unwrap() = app_config;
    }
    std::fs::write(PATH, json).unwrap();
}

fn _get_app_config() -> AppConfig {
    APPCONFIG.lock().unwrap().clone()
}

#[tauri::command]
fn get_app_config() -> AppConfig {
    _get_app_config()
}

#[tauri::command]
fn write_app_config(app_config: AppConfig) {
    _write_app_config(app_config);
}

fn app_menu() -> Menu {
    Menu::with_items([
        MenuEntry::Submenu(Submenu::new(
            "File",
            Menu::with_items([
                CustomMenuItem::new("new", "New")
                    .accelerator("CmdOrCtrl+N")
                    .into(),
                CustomMenuItem::new("open", "Open")
                    .accelerator("CmdOrCtrl+O")
                    .into(),
                CustomMenuItem::new("save", "Save")
                    .accelerator("CmdOrCtrl+S")
                    .into(),
                MenuItem::Separator.into(),
                CustomMenuItem::new("quit", "Quit")
                    .accelerator("CmdOrCtrl+Q")
                    .into(),
            ]),
        )),
        MenuEntry::Submenu(Submenu::new(
            "Tools",
            Menu::with_items([CustomMenuItem::new("wall", "Wall").accelerator("W").into()]),
        )),
    ])
}

fn main() {
  load_app_config();
    tauri::Builder::default()
        // .on_page_load(|window, _payload| {
        //   let payload = BootPayload { drives: scan_drive() };
        //   window
        //     .emit("boot", Some(payload))
        //     .expect("failed to emit event");
        // })
        .menu(app_menu())
        .on_menu_event(|event| {
            let app_handle = event.window().app_handle();
            match event.menu_item_id() {
                "new" => {
                    app_handle
                    .emit_all("new", ())
                    .expect("Failed to send event");
                },
                "open" => {
                    app_handle
                        .emit_all("open", ())
                        .expect("Failed to send event");
                }
                "save" => {
                    app_handle
                        .emit_all("save", ())
                        .expect("Failed to send event");
                }
                "wall" => {
                    app_handle
                        .emit_all("wall", ())
                        .expect("Failed to send event");
                }
                "quit" => std::process::exit(0),
                _ => {}
            }
        })
        .on_page_load(|window, payload| {})
        .setup(|_| {
            tauri::async_runtime::spawn(do_montecarlo());
            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            room_state::should_play,
            room_state::get_config,
            heatmap::get_active_best,
            room_state::save_config,
            room_state::get_bb,
            get_app_config,
            write_app_config
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
