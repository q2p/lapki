// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

// Learn more about Tauri commands at https://tauri.app/v1/guides/features/command
#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Rust!", name)
}

fn main() {
  tauri::Builder::default()
    .invoke_handler(tauri::generate_handler![greet])
    .run(tauri::generate_context!())
    .expect("error while running tauri application");

  tauri::Builder::default()
    .manage()
    .manage(DriveEntries(scan_drive().into()))
    .manage(ActiveDrive(0.into()))
    .manage(ActivePath(Default::default()))
    .manage(DirEntries(Default::default()))
    .manage(SubDirectoriesCount(Default::default()))
    .invoke_handler(tauri::generate_handler![
      change_drive,
      scan_dir,
      change_dir,
      count_sub_dir,
    ])
    .register_uri_scheme_protocol("reqimg", move |app, request| {
      let res_not_img = ResponseBuilder::new().status(404).body(Vec::new());
      if request.method() != "GET" { return res_not_img; }
      let uri = request.uri();
      let start_pos = match uri.find("?n=") {
        Some(_pos) => _pos + 3,
        None => return res_not_img,
      };
      let end_pos = match uri.find("&") {
        Some(_pos) => _pos,
        None => return res_not_img,
      };
      let entry_num: usize = match &uri[start_pos..end_pos].parse() {
        Ok(_i) => *_i,
        Err(_) => return res_not_img,
      };
      let dir_entries: State<DirEntries> = app.state();
      let v_dirs = &*dir_entries.0.lock().unwrap();
      let target_file = match v_dirs.get(entry_num) {
        Some(_dir) => &v_dirs[entry_num],
        None => return res_not_img,
      };
      let extension = match target_file.extension() {
        Some(_ex) => _ex.to_string_lossy().to_string(),
        None => return res_not_img,
      };
      if !is_img_extension(&extension) {
        return res_not_img;
      }
      println!("🚩Request: {} / {:?}", entry_num, target_file);
      let local_img = if let Ok(data) = read(target_file) {
        tauri::http::ResponseBuilder::new()
          .mimetype(format!("image/{}", &extension).as_str())
          .body(data)
      } else {
        res_not_img
      };
      local_img
    })
    .on_page_load(|window, _payload| {
      let payload = BootPayload { drives: scan_drive() };
      window
        .emit("boot", Some(payload))
        .expect("failed to emit event");
    })
    .run(tauri::generate_context!())
    .expect("error while running tauri application");
}
