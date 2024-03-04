import { exit } from "@tauri-apps/api/process";
import { AppState, Config, DrawingState, Wall } from "./types";
import { save as save_dialog } from '@tauri-apps/api/dialog';
import { appWindow } from "@tauri-apps/api/window";
import { save_config } from "./api";

export const quit = async () => {
    await exit(1);
}

export const save = async (app_state: AppState, drawing_state: DrawingState) => {
    if (app_state.config_path) {
        app_state.config.walls = [...app_state.config.walls, ...drawing_state.added_walls]
    } else {
        //fix me
        app_state.config = <Config> {
            walls: drawing_state.added_walls,
            radio_points: [],
            radio_zones: []
        }
        const filePath = await save_dialog({
            filters: [{
                name: 'Config',
                extensions: ['json']
            }]
        });
        app_state.config_path = filePath
    }
    app_state.changes = false
    save_config(app_state.config, app_state.config_path)
    appWindow.setTitle("[" + app_state.config_path.split('\\').pop() + "] – " + "5G Planner ")
}

export const open = async () => {

}

export const newConfig = async (app_state: AppState) => {
    app_state.config.walls = []
    app_state.config.radio_points = []
    app_state.config.radio_zones = []
    app_state.changes = false
    await appWindow.setTitle("[  ] – " + "5G Planner ")
    app_state.config_path = ""
}