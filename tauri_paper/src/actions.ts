import { exit } from "@tauri-apps/api/process"
import { AppState, Config, DrawingState, Wall } from "./types"
import { save as save_dialog, open } from "@tauri-apps/api/dialog"
import { appWindow } from "@tauri-apps/api/window"
import { get_config, save_config, write_app_config } from "./api"
import { dialog } from "@tauri-apps/api"
import { center_view } from "./main"

//  Project File Actions

export const quit = async () => {
  await exit(1)
}

export const save = async (app_state: AppState, drawing_state: DrawingState) => {
  if (app_state.config_path) {
    app_state.config.walls = [...app_state.config.walls, ...drawing_state.added_walls]
  } else {
    // fix me
    app_state.config = <Config> {
      walls: drawing_state.added_walls,
      radio_points: [],
      radio_zones: [],
    }
    const filePath = await save_dialog({
      filters: [{
        name: "Config",
        extensions: ["json"],
      }],
    })
    app_state.config_path = filePath
  }
  app_state.changes = false
  save_config(app_state.config, app_state.config_path)
  appWindow.setTitle("[" + app_state.config_path.split("\\").pop() + "] – " + "5G Planner ")
}

export const openConfig = async (app_state: AppState) => {
  const selected = await open({
    multiple: false,
    filters: [{
      name: "Config",
      extensions: ["json"],
    }],
  })
  app_state.config_path = selected
  app_state.config = await get_config(app_state.config_path)
  await appWindow.setTitle("[" + app_state.config_path.split("\\").pop() + "] – " + "5G Planner ")
  center_view()
}

export const newConfig = async (app_state: AppState) => {
  app_state.config.walls = []
  app_state.config.radio_points = []
  app_state.config.radio_zones = []
  app_state.changes = false
  await appWindow.setTitle("[  ] – " + "5G Planner ")
  app_state.config_path = ""
}

export const close = async (app_state: AppState, drawing_state: DrawingState) => {
  if (app_state.changes) {
    if (app_state.config_path) {
      const result = await dialog.confirm("There are unsaved changes to " + app_state.config_path.split("/").pop(), { title: "Do you want to save your work", type: "warning" })
      if (!result) {
        return
      } else {
        app_state.config.walls = [...app_state.config.walls, ...drawing_state.added_walls]
        await save_config(app_state.config, app_state.config_path)
      }
      app_state.app_config.latest_config = app_state.config_path
      await write_app_config(app_state.app_config)
    } else {
      const result = await dialog.confirm("There are unsaved changes to Untitled", { title: "Do you want to save your work", type: "warning" })
      if (!result) {
        return
      } else {
        app_state.config = <Config>{
          walls: drawing_state.added_walls,
          radio_points: [],
          radio_zones: [],
        }
        const filePath = await save_dialog({
          filters: [{
            name: "Config",
            extensions: ["json"],
          }],
          title: "Choice file to save your work to",
        })
        if (!filePath) {
          return
        }
        await save_config(app_state.config, filePath)
        app_state.app_config.latest_config = filePath
        await write_app_config(app_state.app_config)
      }
    }
  } else {
    if (app_state.config_path) {
      app_state.app_config.latest_config = app_state.config_path
      await write_app_config(app_state.app_config)
    }
  }
}
