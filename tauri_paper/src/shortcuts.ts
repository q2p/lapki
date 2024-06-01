// global hotkeys

import { register } from "@tauri-apps/api/globalShortcut"
import { newConfig, openConfig, quit, save } from "./actions"
import { AppState, DrawingState } from "./types"

export const registerGlobalShortcuts = async (app_state: AppState, drawing_state: DrawingState) => {
  await register("CommandOrControl+N", async () => {
    await newConfig(app_state)
  })

  await register("CommandOrControl+O", async () => {
    await openConfig(app_state)
  })

  await register("CommandOrControl+S", async () => {
    await save(app_state, drawing_state)
  })

  await register("CommandOrControl+Q", async () => {
    await quit()
  })
}
