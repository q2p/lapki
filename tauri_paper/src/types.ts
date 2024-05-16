export type AppConfig = {
    latest_config?: string,
}

export type Config = {
    walls: Wall[],
    radio_points: RadioPoint[],
    radio_zones: RadioZone[],
};

export type Wall = {
    a: Point2d,
    b: Point2d,
    damping: number,
};

export type RadioPoint = {
    pos: Point2d,
    power: number,
};

export type RadioZone = {
    points: Point2d[],
    r: number,
    g: number,
    b: number,
};

export type Point2d = {
    x: number,
    y: number,
};

export type ActiveBest = {
    point_x: number,
    point_y: number,
    point_pow_mw: number,
    min_sinr_dbm: number,
    min_sinr_x: number,
    min_sinr_y: number,
    r: number,
    g: number,
    b: number,
};

export type AppState = {
    app_config: AppConfig,
    config: Config,
    config_path: string,
    changes: boolean,
}

export type DrawingState = {
    drawing: boolean,
    mevent: MouseEvent,
    drawing_last_point: Point2d | null,
    drawing_point: Point2d | null,
    added_walls: Wall[],
}

export type BoundingBoxes = {
    min: Point2d,
    max: Point2d,
    wh: Point2d,
    res: [number, number]
}

export class StoredBest {
  constructor(
        readonly x: number,
        readonly y: number,
        readonly tooltip: HTMLDivElement,
        readonly circle: HTMLDivElement,
  ) { }
}
