import { TauriEvent, UnlistenFn, listen } from "@tauri-apps/api/event"
import { AppState, Config, DrawingState } from "./types"
import { appWindow } from "@tauri-apps/api/window"
import { dialog } from "@tauri-apps/api"
import { open, save as save_dialog } from "@tauri-apps/api/dialog"
import { close, newConfig, openConfig, quit, save } from "./actions"
import { get_config, save_config, write_app_config } from "./api"

let
  unlisten_notification: UnlistenFn,
  unlisten_new: UnlistenFn,
  unlisten_open: UnlistenFn,
  unlisten_save: UnlistenFn,
  unlisten_wall: UnlistenFn,
  unlisten_close: UnlistenFn

export const registerGlobalListeners = async (app_state: AppState, drawing_state: DrawingState) => {
  unlisten_notification = await listen("notification", (event) => {
    dialog.message(event.payload.message, { type: "info" })
  })
  unlisten_new = await listen("new", async () => {
    await newConfig(app_state)
  })
  unlisten_open = await listen("open", async () => {
    await openConfig(app_state)
  })
  unlisten_save = await listen("save", async () => {
    await save(app_state, drawing_state)
  })
  unlisten_wall = await listen("wall", () => {
    if (drawing_state.drawing) {
      drawing_state.drawing = false
    } else {
      drawing_state.drawing = true
    }
  })
  unlisten_close = await appWindow.listen(TauriEvent.WINDOW_CLOSE_REQUESTED, async () => {
    await close(app_state, drawing_state)

    unlisten_notification()
    unlisten_open()
    unlisten_save()
    unlisten_wall()
    unlisten_close()
    unlisten_new()

    await quit()
  })
}
