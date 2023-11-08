// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use std::fs::File;
use std::path::Path;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Mutex;
use std::thread;
use std::time::Duration;

use random_tries::do_montecarlo;
use serde::{Deserialize, Serialize};
use tauri::api::notification::Notification;
use tauri::async_runtime::{handle, JoinHandle};
use tauri::{Manager, App};
use tokio::sync::mpsc;
use tokio::time::{sleep, sleep_until};
use tokio_util::sync::CancellationToken;

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

fn main() {
    // room_state::load_config("dasdfsadsad");
    load_app_config();
    tauri::Builder::default()
        // .on_page_load(|window, _payload| {
        //   let payload = BootPayload { drives: scan_drive() };
        //   window
        //     .emit("boot", Some(payload))
        //     .expect("failed to emit event");
        // })
        .on_page_load(|window, payload| {})
        .setup(|app| {
            let product_name = app.config().package.product_name.clone().unwrap();
            let app_handle = app.handle();
            let id_run_event = app.listen_global("run", move |_| {
                if RUNNING.load(Ordering::Relaxed) == true {
                    // Notification::new("tauri_paper")
                    //   .title("Info")
                    //   .body("Simulation already running")
                    //   .show()
                    //   .expect("Failed send notification");
                    app_handle
                        .emit_all(
                            "notification",
                            NotificationPayload {
                                message: "Simulation already running!".into(),
                            },
                        )
                        .expect("Failed to send event");
                    return;
                }
                RUNNING.store(true, Ordering::SeqCst);
            });
            let app_handle = app.handle();
            let id_stop_event = app.listen_global("stop", move |_| {
                if RUNNING.load(Ordering::SeqCst) == true {
                    RUNNING.store(false, Ordering::SeqCst);
                    return;
                }
                app_handle
                    .emit_all(
                        "notification",
                        NotificationPayload {
                            message: "No simulations running!".into(),
                        },
                    )
                    .expect("Failed to send event");
            });
            tauri::async_runtime::spawn(async move {
                loop {
                    if !RUNNING.load(Ordering::SeqCst) {
                        tokio::time::sleep(Duration::from_millis(50)).await;
                        continue;
                    }
                    do_montecarlo().await;
                }
            });
            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            room_state::get_config,
            heatmap::get_active_best,
            room_state::save_config,
            get_app_config,
            write_app_config
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
