import { invoke } from "@tauri-apps/api";
import { ActiveBest, AppConfig, Config } from "./types";

export async function get_config(path: string): Promise<Config> {
    return (await invoke("get_config", { path: path })) as Config;
}

export async function save_config(config: Config, path: string) {
    invoke('save_config', { config: config, path: path })
}

export async function get_app_config(): Promise<AppConfig> {
    return (await invoke("get_app_config")) as AppConfig
}

export async function write_app_config(app_config: AppConfig) {
    await invoke("write_app_config", { appConfig: app_config })
}

export async function get_active_best(): Promise<ActiveBest[]> {
    return (await invoke("get_active_best")) as ActiveBest[];
}