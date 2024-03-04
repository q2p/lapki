import { TauriEvent, UnlistenFn, listen } from "@tauri-apps/api/event";
import { AppState, Config, DrawingState } from "./types";
import { appWindow } from "@tauri-apps/api/window";
import { dialog } from "@tauri-apps/api";
import { open, save as save_dialog } from '@tauri-apps/api/dialog';
import { newConfig, quit, save } from "./actions";
import { get_config, save_config, write_app_config } from "./api";

let 
    unlisten_notification: UnlistenFn, 
    unlisten_new: UnlistenFn, 
    unlisten_open: UnlistenFn, 
    unlisten_save: UnlistenFn, 
    unlisten_wall: UnlistenFn, 
    unlisten_close: UnlistenFn

export const registerGlobalListeners = async (app_state: AppState, drawing_state: DrawingState) => {
    unlisten_notification = await listen('notification', (event) => {
        dialog.message(event.payload.message, { type: "info" })
    })
    unlisten_new = await listen('new', async () => {
        await newConfig(app_state)
    })
    unlisten_open = await listen('open', async () => {
        const selected = await open({
            multiple: false,
            filters: [{
                name: 'Config',
                extensions: ['json']
            }]
        });
        app_state.config_path = selected
        app_state.config = await get_config(app_state.config_path)
        await appWindow.setTitle("[" + app_state.config_path.split('\\').pop() + "] – " + "5G Planner ")
    })
    unlisten_save = await listen('save', async () => {
        await save(app_state, drawing_state)
    })
    unlisten_wall = await listen('wall', () => {
        if (drawing_state.drawing) {
            drawing_state.drawing = false;
        } else {
            drawing_state.drawing = true;
        }
    })
    unlisten_close = await appWindow.listen(TauriEvent.WINDOW_CLOSE_REQUESTED, async () => {
        console.log("close req")
        if (app_state.changes) {
            if (app_state.config_path) {
                let result = await dialog.confirm("There are unsaved changes to " + app_state.config_path.split("/").pop(), { title: 'Do you want to save your work', type: "warning" })
                if (!result) {
                    return;
                } else {
                    app_state.config.walls = [...app_state.config.walls, ...drawing_state.added_walls]
                    await save_config(app_state.config, app_state.config_path)
                }
                app_state.app_config.latest_config = app_state.config_path;
                await write_app_config(app_state.app_config)
            } else {
                let result = await dialog.confirm("There are unsaved changes to Untitled", { title: 'Do you want to save your work', type: "warning" })
                if (!result) {
                    return;
                } else {
                    app_state.config = <Config>{
                        walls: drawing_state.added_walls,
                        radio_points: [],
                        radio_zones: []
                    }
                    const filePath = await save_dialog({
                        filters: [{
                            name: 'Config',
                            extensions: ['json']
                        }],
                        title: 'Choice file to save your work to'
                    });
                    if (!filePath) {
                        return;
                    }
                    await save_config(app_state.config, filePath)
                    app_state.app_config.latest_config = filePath;
                    await write_app_config(app_state.app_config)
                }
            }
        } else {
            if (app_state.config_path) {
                app_state.app_config.latest_config = app_state.config_path;
                await write_app_config(app_state.app_config)
            }
        }

        unlisten_notification()
        unlisten_open()
        unlisten_save()
        unlisten_wall()
        unlisten_close()
        unlisten_new()

        await quit()
    })
}

