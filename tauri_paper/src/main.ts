import { invoke } from "@tauri-apps/api/tauri";

interface Config {
  walls: Wall[];
  radio_points: RadioPoint[];
  radio_zones: RadioZone[];
}

interface Wall {
  a: Point2d;
  b: Point2d;
  damping: number;
}

interface RadioPoint {
  pos: Point2d;
  power: number;
}

interface RadioZone {
  points: Point2d[];
}

interface Point2d {
  x: number;
  y: number;
}

async function get_config(): Promise<Config>{
  return invoke("get_config")
    .then((cfg: Config) => {return cfg} );
}

let config: Config = await get_config();
let canvas: HTMLCanvasElement;
let context: CanvasRenderingContext2D | null;

canvas = document.getElementById('canvas') as HTMLCanvasElement;
context = canvas.getContext("2d");

context?.beginPath();
for (let i = 0; i < config.walls.length; i++) {
  context?.moveTo(config.walls[i].a.x, config.walls[i].a.y);
  context?.lineTo(config.walls[i].b.x, config.walls[i].b.y);
}
context?.stroke();