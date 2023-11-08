import { emit, listen } from "@tauri-apps/api/event";
import { invoke } from "@tauri-apps/api/tauri";
import { open, save as save_dialog } from '@tauri-apps/api/dialog';
import { WebviewWindow } from '@tauri-apps/api/window';
import { appWindow } from '@tauri-apps/api/window';
import { dialog } from "@tauri-apps/api";
import { TauriEvent } from "@tauri-apps/api/event"
import { exit } from "@tauri-apps/api/process";

const style = (node, styles) =>
  Object.keys(styles).forEach((key) => (node.style[key] = styles[key]));

type Config = {
  walls: Wall[];
  radio_points: RadioPoint[];
  radio_zones: RadioZone[];
};

type Wall = {
  a: Point2d;
  b: Point2d;
  damping: number;
};

type RadioPoint = {
  pos: Point2d;
  power: number;
};

type RadioZone = {
  points: Point2d[];
  r: number;
  g: number;
  b: number;
};

type Point2d = {
  x: number;
  y: number;
};

type ActiveBest = {
  point_x: number;
  point_y: number;
  point_pow_mw: number;
  min_sinr_dbm: number;
  min_sinr_x: number;
  min_sinr_y: number;
  r: number;
  g: number;
  b: number;
};

class StoredBest {
  constructor(
    readonly x: number,
    readonly y: number,
    readonly tooltip: HTMLDivElement,
    readonly circle: HTMLDivElement
  ) { }
}

async function get_config(path: string): Promise<Config> {
  return (await invoke("get_config", { path: path })) as Config;
}

async function get_active_best(): Promise<ActiveBest[]> {
  return (await invoke("get_active_best")) as ActiveBest[];
}

// let config: Config = await get_config();
let config: Config
let config_path: string
let changes: boolean = false

// rimg meters
const rimg_xmin = 5;
const rimg_ymin = 12;
const rimg_xmax = 60 - (0 * 55) / 600;
const rimg_ymax = 52 - (0 * 40) / 436;
const rimg_yc = (rimg_ymin + rimg_ymax) / 2;
const rimg_xc = (rimg_xmin + rimg_xmax) / 2;

let camX = rimg_xc;
let camY = rimg_yc;

let cursor_x_m = 0;
let cursor_y_m = 0;
let cursor_x_p = 0;
let cursor_y_p = 0;

//b egorkasprigorca
let drawing: boolean = false;
let mevent: MouseEvent;
let drawing_last_point: Point2d | null = null;
let drawing_point: Point2d | null = null;

let added_walls: Wall[] = [];

//e

const canvas = document.createElement("canvas");
document.body.appendChild(canvas);
const ctx = canvas.getContext("2d")!;

const image = new Image();
setInterval(function () {
  if (!image.complete) {
    image.src = image.src;
  }
}, 200);

const wall_thickness = 8;

let zoom_pow = 1;
let zoom_target = 32;
let zoom = zoom_target;

window.addEventListener("wheel", function (e: WheelEvent) {
  zoom_pow = Math.max(-4, Math.min(6, zoom_pow - e.deltaY / 100));
  zoom_target = 32 * Math.pow(1.5, zoom_pow);
});

window.addEventListener("mousemove", function (e) {
  cursor_x_p = e.clientX;
  cursor_y_p = e.clientY;
  if (is_pressing) {
    camX = anchor_x_m - (cursor_x_p - canvas.width / 2) / zoom;
    camY = anchor_y_m - (cursor_y_p - canvas.height / 2) / zoom;
  }
  cursor_x_m = camX + (cursor_x_p - canvas.width / 2) / zoom;
  cursor_y_m = camY + (cursor_y_p - canvas.height / 2) / zoom;

  mevent = e;
});

let anchor_x_m = 0;
let anchor_y_m = 0;
let is_pressing = false;

let drawing_pressed = false;

window.addEventListener("mousedown", function (e) {
  cursor_x_p = e.clientX;
  cursor_y_p = e.clientY;
  cursor_x_m = camX + (cursor_x_p - canvas.width / 2) / zoom;
  cursor_y_m = camY + (cursor_y_p - canvas.height / 2) / zoom;
  if (drawing) {
    drawing_pressed = true;
    drawing_last_point = {
      x: cursor_x_m,
      y: cursor_y_m,
    };
  } else {
    is_pressing = true;
    anchor_x_m = cursor_x_m;
    anchor_y_m = cursor_y_m;
  }
});

window.addEventListener("mouseup", function (e) {
  cursor_x_p = e.clientX;
  cursor_y_p = e.clientY;
  cursor_x_m = camX + (cursor_x_p - canvas.width / 2) / zoom;
  cursor_y_m = camY + (cursor_y_p - canvas.height / 2) / zoom;
  if (drawing) {
    if (
      drawing_last_point &&
      drawing_point &&
      Math.abs(cursor_x_m - drawing_last_point.x) <= 0.9 &&
      Math.abs(cursor_y_m - drawing_last_point.y) <= 0.9
    ) {
      drawing_pressed = false;
      drawing_last_point = null;
    } else {
      drawing_pressed = false;
      drawing_point = {
        x: cursor_x_m,
        y: cursor_y_m,
      };
      const wall: Wall = {
        a: drawing_last_point!,
        b: drawing_point!,
        damping: 500, //change me
      };
      added_walls.push(wall);
      changes = true;
      if (config_path) {
        appWindow.setTitle("5G Planner " + config_path + " *")
      } else {
        appWindow.setTitle("5G Planner " + "*")
      }
    }
  } else {
    is_pressing = false;
  }
});

window.addEventListener("keydown", function (e) {
  if (e.code === "KeyS") {
    image.src = "../rimg3.png";
    camX = rimg_xc;
    camY = rimg_yc;
    zoom_target = Math.max(
      canvas.width / (rimg_xmax - rimg_xmin),
      canvas.height / (rimg_ymax - rimg_ymin)
    );
    get_active_best().then(update_active_els);
    zoom_pow = Math.log(zoom_target / 32) / Math.log(1.5);
  }
});

function mk_tooltip(x: number, y: number, color: string, text: string) {
  const img = document.createElement("div");
  img.className = "legend_point";
  img.style.position = "absolute";
  img.style.border = "2px solid black";
  img.style.borderRadius = "100%";
  img.style.width = `${img_wh}px`;
  img.style.height = `${img_wh}px`;
  img.style.backgroundColor = color;

  let box = document.createElement("div");
  box.className = "legend_tooltip";
  box.textContent = text;
  box.style.position = "absolute";
  box.style.background = "rgba(33,33,33,0.7)";
  box.style.color = "white";
  box.style.borderRadius = "5px";

  document.body.appendChild(box);
  document.body.appendChild(img);

  elements.push(new StoredBest(x, y, box, img));
}

function update_active_els(a: ActiveBest[]) {
  for (const el of elements) {
    el.circle.remove();
    el.tooltip.remove();
  }
  for (const el of a) {
    const color = ac(el.r, el.g, el.b);
    mk_tooltip(
      el.point_x,
      el.point_y,
      color,
      `Pow: ${el.point_pow_mw.toPrecision(4)}mw`
    );
    mk_tooltip(
      el.min_sinr_x,
      el.min_sinr_y,
      color,
      `Min SINR: ${el.min_sinr_dbm.toPrecision(4)}dbm`
    );
  }
}

function grid(
  offsetX: number,
  offsetY: number,
  grid_size_px: number,
  alpha: number
) {
  ctx.strokeStyle = "#000";
  ctx.lineWidth = 1;
  ctx.beginPath();
  for (let iy = -1; iy !== Math.floor(canvas.height / grid_size_px) + 2; iy++) {
    const ypos = Math.round(
      iy * grid_size_px - ((offsetY * zoom) % grid_size_px)
    );
    ctx.moveTo(0, ypos);
    ctx.lineTo(canvas.width, ypos);
  }
  for (let ix = -1; ix !== Math.floor(canvas.width / grid_size_px) + 2; ix++) {
    const xpos = Math.round(
      ix * grid_size_px - ((offsetX * zoom) % grid_size_px)
    );
    ctx.moveTo(xpos, 0);
    ctx.lineTo(xpos, canvas.height);
  }
  ctx.globalAlpha = alpha;
  ctx.stroke();
  ctx.globalAlpha = 1.0;
}

function legend(offsetX: number, offsetY: number) {
  let i = 0;
  for (const el of elements) {
    let wh, text_size_base;
    const past_zoom = Math.max(1, zoom / 100);
    wh = img_wh * past_zoom;
    text_size_base = 15 * past_zoom;

    const xpos = Math.round(zoom * (el.x - offsetX) - wh / 2);
    const ypos = Math.round(zoom * (el.y - offsetY) - wh / 2);

    el.circle.style.left = `${Math.round(xpos)}px`;
    el.circle.style.top = `${Math.round(ypos)}px`;
    el.circle.style.width = `${wh}px`;
    el.circle.style.height = `${wh}px`;

    el.tooltip.style.left = `${xpos}px`;
    el.tooltip.style.top = `${ypos + wh * 1.2}px`;
    el.tooltip.style.transform = "translateX(-50%)";
    el.tooltip.style.fontFamily = `sans-serif`;
    el.tooltip.style.fontSize = `${text_size_base}px`;
    el.tooltip.style.paddingLeft = `${(text_size_base * 6) / 15}px`;
    el.tooltip.style.paddingRight = `${(text_size_base * 6) / 15}px`;
    el.tooltip.style.paddingTop = `${(text_size_base * 2) / 15}px`;
    el.tooltip.style.paddingBottom = `${(text_size_base * 2) / 15}px`;

    i++;
  }
}

function zc(i: number) {
  const z = config.radio_zones[i];
  return ac(z.r, z.g, z.b);
}

function ac(r: number, g: number, b: number) {
  return "#" + (0x1000000 + r * 256 * 256 + g * 256 + b).toString(16).slice(-6);
}

let elements: StoredBest[] = [];

// let img = document.getElementById('rimg');
const img_wh = 12;

function raf() {
  requestAnimationFrame(raf);

  ctx.imageSmoothingEnabled = true;
  ctx.imageSmoothingQuality = "high";

  // const t = new Date().getTime()

  zoom = zoom_target * 0.1 + zoom * 0.9;
  if (Math.abs(zoom - zoom_target) < 0.001) {
    zoom = zoom_target;
  }

  if (is_pressing) {
    camX = anchor_x_m - (cursor_x_p - canvas.width / 2) / zoom;
    camY = anchor_y_m - (cursor_y_p - canvas.height / 2) / zoom;
  }

  // let camX = rimg_xc+16*Math.sin(t*0.0001);
  // let camY = rimg_yc+16*Math.cos(t*0.0001);
  // let zoom = 24+8*Math.cos(1253.0+t*0.0001);
  // zoom = 32
  let offsetX = camX - canvas.width / (2 * zoom);
  let offsetY = camY - canvas.height / (2 * zoom);

  ctx.fillStyle = "#fff";
  ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);

  if (image.complete) {
    const dx = Math.round((rimg_xmin - offsetX) * zoom);
    const dy = Math.round((rimg_ymin - offsetY) * zoom);
    const rx = Math.round((rimg_xmax - rimg_xmin) * zoom);
    const ry = Math.round((rimg_ymax - rimg_ymin) * zoom);
    ctx.drawImage(image, dx, dy, rx, ry);
    ctx.fillRect(0, 0, dx, canvas.height);
    ctx.fillRect(dx + rx, 0, canvas.width, canvas.height);
    ctx.fillRect(dx, 0, rx, dy);
    ctx.fillRect(dx, dy + ry, rx, canvas.height);
  } else {
    ctx.fillRect(0, 0, canvas.width, canvas.height);
  }

  if (zoom >= 128) {
    grid(offsetX, offsetY, zoom / 10, 0.5);
  }
  if (zoom >= 8) {
    grid(offsetX, offsetY, zoom, 0.7);
  }

  //////////////////////////////

  legend(offsetX, offsetY);
  // for (let i = 0; i < tooltips.length - 1; i++) {
  //   let curr = tooltips[i]
  //   let next = tooltips[i+1]
  //   let curr_rect = curr.getBoundingClientRect()
  //   let next_rect = curr.getBoundingClientRect()

  //   if (  !(curr_rect.top > next_rect.bottom ||
  //         curr_rect.right < next_rect.left ||
  //         curr_rect.bottom < next_rect.top ||
  //         curr_rect.left > next_rect.right)
  //   ){
  //     let overlap_x = 0
  //     let overlap_y = 0
  //     if (curr_rect.top > next_rect.bottom) {
  //       overlap_y = Math.abs(curr_rect.top - next_rect.bottom)
  //     }
  //     if (curr_rect.right < next_rect.left) {
  //       overlap_x = Math.abs(curr_rect.right - next_rect.left)
  //     }
  //     if (curr_rect.bottom < next_rect.top) {
  //       // overlap_y = Math.abs(curr_rect.bottom - next_rect.top)
  //     }
  //     if (curr_rect.left > next_rect.right) {
  //       // overlap_x = Math.abs(curr_rect.left - next_rect.right)
  //     }

  //     curr.style.top = `${curr.offsetTop - overlap_y / 2}px`
  //     next.style.top = `${curr.offsetTop + overlap_y / 2}px`
  //     curr.style.left = `${curr.offsetLeft - overlap_x / 2}px`
  //     next.style.left = `${curr.offsetLeft + overlap_x / 2}px`
  //    }
  // }

  /////////////////////////////////////////

  ctx.lineWidth = wall_thickness;
  ctx.strokeStyle = "#000";
  ctx.fillStyle = "#000";
  ctx.beginPath();
  for (const w of config.walls) {
    const xmin = Math.round((Math.min(w.a.x, w.b.x) - offsetX) * zoom);
    const xmax = Math.round((Math.max(w.a.x, w.b.x) - offsetX) * zoom);
    const ymin = Math.round((Math.min(w.a.y, w.b.y) - offsetY) * zoom);
    const ymax = Math.round((Math.max(w.a.y, w.b.y) - offsetY) * zoom);
    ctx.fillRect(
      xmin - wall_thickness / 2,
      ymin - wall_thickness / 2,
      xmax - xmin + wall_thickness,
      ymax - ymin + wall_thickness
    );

    // Раскомментить для угловатых стен.
    // w.a.x = Math.round(w.a.x)
    // w.a.y = Math.round(w.a.y)
    // w.b.x = Math.round(w.b.x)
    // w.b.y = Math.round(w.b.y)
    // ctx.moveTo((w.a.x-offsetX)*zoom, (w.a.y-offsetY)*zoom);
    // ctx.lineTo((w.b.x-offsetX)*zoom, (w.b.y-offsetY)*zoom);
  }
  ctx.stroke();

  const scales = [1, 2, 5];
  for (let i = -10; i !== 32; i++) {
    const decs = Math.floor(i / 3);
    const scale = scales[((i % 3) + 3) % 3];
    const meters = Math.pow(10, decs) * scale;
    if (meters * zoom > 64) {
      ctx.lineWidth = 1;
      ctx.strokeStyle = "#000";
      ctx.fillStyle = "#000";

      ctx.font = "bold 22px sans-serif";
      const width = Math.round(meters * zoom);
      ctx.textAlign = "center";
      ctx.fillText(`${meters}m`, 32 + width / 2, canvas.height - 50);
      ctx.fillRect(32, canvas.height - 34, width, 2);
      ctx.fillRect(32, canvas.height - 37, 2, 8);
      ctx.fillRect(32 + width, canvas.height - 37, 2, 8);
      break;
    }
  }
}

function resize() {
  console.log("onresize");
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
}

const unlisten = await listen('notification', (event) => {
  dialog.message(event.payload.message, { type: "info" })
})

appWindow.listen(TauriEvent.WINDOW_CLOSE_REQUESTED, async () => {
  console.log("close req")
  if (changes) {
    if (config_path) {
      let result = await dialog.confirm("There are unsaved changes to " + config_path.split("/").pop(), { title: 'Do you want to save your work', type: "warning" })
      if (!result) {
        return;
      } else {
        config.walls = [...config.walls, ...added_walls]
        await invoke('save_config', { config: config, path: config_path })
      }
    } else {
      let result = await dialog.confirm("There are unsaved changes to Untitled", { title: 'Do you want to save your work', type: "warning" })
      if (!result) {
        return;
      } else {
        config = <Config>{
          walls: added_walls,
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
        await invoke('save_config', { config: config, path: filePath })
      }
    }
  }
  await exit(1);
})

const container = document.createElement("div")
style(container, {
  position: "absolute",
  top: "10px",
  left: "10px",
  display: "flex",
  gap: "10px"
})
const tools = document.createElement("div");
tools.classList.add("menu-item")
tools.innerText = "Tools";
tools.addEventListener("click", (e) => {
  if (drawing) {
    drawing = false;
  } else {
    drawing = true;
  }
});
const load = document.createElement("div")
load.classList.add("menu-item")
load.innerText = "Load"
load.addEventListener("click", async (e) => {
  const selected = await open({
    multiple: false,
    filters: [{
      name: 'Config',
      extensions: ['json']
    }]
  });
  config_path = selected
  config = await get_config(config_path)
  await appWindow.setTitle("5G Planner " + selected)
})
const save = document.createElement("div")
save.classList.add("menu-item")
save.innerText = "Save";
save.addEventListener("click", async (e) => {
  if (config_path) {
    config.walls = [...config.walls, ...added_walls]
  } else {
    //fix me
    config = <Config>{
      walls: added_walls,
      radio_points: [],
      radio_zones: []
    }
    const filePath = await save_dialog({
      filters: [{
        name: 'Config',
        extensions: ['json']
      }]
    });
    config_path = filePath
  }
  changes = false
  invoke('save_config', { config: config, path: config_path })
  appWindow.setTitle("5G Planner " + config_path)
});
const run = document.createElement("div")
run.classList.add("menu-item")
run.innerText = "Run";
run.addEventListener("click", async (e) => {
  emit('run')
});
const stop = document.createElement("div")
stop.classList.add("menu-item")
stop.innerText = "Stop";
stop.addEventListener("click", (e) => {
  emit('stop')
});
container.appendChild(tools)
container.appendChild(load)
container.appendChild(save)
container.appendChild(run)
container.appendChild(stop)
// canvas.addEventListener('mousemove', (e) => {
//   if (drawing) {
//     console.log(e.offsetX - , e.offsetY)
//     var coord = {
//       'x': e.offsetX,
//       'y': e.offsetY
//     };
//     if (prev !== undefined) {
//       ctx.beginPath();
//       ctx.moveTo(prev.x, prev.y);
//       ctx.lineTo(coord.x, coord.y);
//       ctx.stroke();
//     }
//     prev = coord;
//   }
// })
document.body.appendChild(container);

export function raf2() {
  requestAnimationFrame(raf2);

  ctx.imageSmoothingEnabled = true;
  ctx.imageSmoothingQuality = "high";

  // const t = new Date().getTime()

  zoom = zoom_target * 0.1 + zoom * 0.9;
  if (Math.abs(zoom - zoom_target) < 0.001) {
    zoom = zoom_target;
  }

  // let camX = rimg_xc+16*Math.sin(t*0.0001);
  // let camY = rimg_yc+16*Math.cos(t*0.0001);
  // let zoom = 24+8*Math.cos(1253.0+t*0.0001);
  // zoom = 32
  let offsetX = camX - canvas.width / (2 * zoom);
  let offsetY = camY - canvas.height / (2 * zoom);

  ctx.fillStyle = "#fff";
  ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);

  if (zoom >= 128) {
    grid(offsetX, offsetY, zoom / 10, 0.5);
  }
  if (zoom >= 8) {
    grid(offsetX, offsetY, zoom, 0.7);
  }

  if (config !== undefined) {
    ctx.lineWidth = wall_thickness
    ctx.strokeStyle = "#000";
    ctx.fillStyle = "#000"
    ctx.beginPath();
    for (const w of config.walls) {
      const xmin = Math.round((Math.min(w.a.x, w.b.x) - offsetX) * zoom)
      const xmax = Math.round((Math.max(w.a.x, w.b.x) - offsetX) * zoom)
      const ymin = Math.round((Math.min(w.a.y, w.b.y) - offsetY) * zoom)
      const ymax = Math.round((Math.max(w.a.y, w.b.y) - offsetY) * zoom)
      ctx.fillRect(xmin - wall_thickness / 2, ymin - wall_thickness / 2, xmax - xmin + wall_thickness, ymax - ymin + wall_thickness)

      // Раскомментить для угловатых стен.
      // w.a.x = Math.round(w.a.x)
      // w.a.y = Math.round(w.a.y)
      // w.b.x = Math.round(w.b.x)
      // w.b.y = Math.round(w.b.y)
      // ctx.moveTo((w.a.x-offsetX)*zoom, (w.a.y-offsetY)*zoom);
      // ctx.lineTo((w.b.x-offsetX)*zoom, (w.b.y-offsetY)*zoom);
    }
    ctx.stroke();
  }

  if (added_walls.length > 0) {
    ctx.lineWidth = wall_thickness;
    ctx.strokeStyle = "#000";
    ctx.fillStyle = "#000";
    for (const w of added_walls) {
      const xmin = Math.round((Math.min(w.a.x, w.b.x) - offsetX) * zoom);
      const xmax = Math.round((Math.max(w.a.x, w.b.x) - offsetX) * zoom);
      const ymin = Math.round((Math.min(w.a.y, w.b.y) - offsetY) * zoom);
      const ymax = Math.round((Math.max(w.a.y, w.b.y) - offsetY) * zoom);
      ctx.fillRect(
        xmin - wall_thickness / 2,
        ymin - wall_thickness / 2,
        xmax - xmin + wall_thickness,
        ymax - ymin + wall_thickness
      );
    }
  }

  if (drawing) {
    ctx.strokeStyle = "#000";
    ctx.fillStyle = "#000";
    ctx.beginPath();
    ctx.fillRect(
      mevent.clientX - wall_thickness / 2,
      mevent.clientY - wall_thickness / 2,
      wall_thickness,
      wall_thickness
    );
    ctx.stroke();

    if (drawing_last_point) {
      const x = Math.round((drawing_last_point.x - offsetX) * zoom);
      const y = Math.round((drawing_last_point.y - offsetY) * zoom);
      ctx.strokeStyle = "#000";
      ctx.fillStyle = "#000";
      ctx.beginPath();
      ctx.fillRect(
        x - wall_thickness / 2,
        y - wall_thickness / 2,
        wall_thickness,
        wall_thickness
      );
      ctx.stroke();
      if (drawing_point) {
        const x = Math.round((drawing_point.x - offsetX) * zoom);
        const y = Math.round((drawing_point.y - offsetY) * zoom);
        ctx.strokeStyle = "#000";
        ctx.fillStyle = "#000";
        ctx.beginPath();
        ctx.fillRect(
          x - wall_thickness / 2,
          y - wall_thickness / 2,
          wall_thickness,
          wall_thickness
        );
        ctx.stroke();
      }
      if (drawing_pressed) {
        ctx.lineWidth = wall_thickness / 4;
        ctx.strokeStyle = "#000";
        ctx.fillStyle = "#000";
        ctx.beginPath();
        ctx.moveTo(
          (drawing_last_point.x - offsetX) * zoom,
          (drawing_last_point.y - offsetY) * zoom
        );
        ctx.lineTo(mevent.offsetX, mevent.offsetY);
        ctx.stroke();
      }
    }
  }

  // ctx.lineWidth = wall_thickness;
  // ctx.strokeStyle = "#000";
  // ctx.fillStyle = "#000";
  // ctx.beginPath();
  // const xmin = (0 - offsetX) * zoom;
  // const xmax = (10 - offsetX) * zoom;
  // const ymin = (0 - offsetY) * zoom;
  // const ymax = (0.001 - offsetY) * zoom;
  // ctx.fillRect(
  //   xmin - wall_thickness / 2,
  //   ymin - wall_thickness / 2,
  //   xmax - xmin + wall_thickness,
  //   ymax - ymin + wall_thickness
  // );
  // ctx.stroke();

  const scales = [1, 2, 5];
  for (let i = -10; i !== 32; i++) {
    const decs = Math.floor(i / 3);
    const scale = scales[((i % 3) + 3) % 3];
    const meters = Math.pow(10, decs) * scale;
    if (meters * zoom > 64) {
      ctx.lineWidth = 1;
      ctx.strokeStyle = "#000";
      ctx.fillStyle = "#000";

      ctx.font = "bold 22px sans-serif";
      const width = Math.round(meters * zoom);
      ctx.textAlign = "center";
      ctx.fillText(`${meters}m`, 32 + width / 2, canvas.height - 50);
      ctx.fillRect(32, canvas.height - 34, width, 2);
      ctx.fillRect(32, canvas.height - 37, 2, 8);
      ctx.fillRect(32 + width, canvas.height - 37, 2, 8);
      break;
    }
  }
}

window.addEventListener("resize", resize);
document.addEventListener("resize", resize);
resize();
// requestAnimationFrame(raf)
requestAnimationFrame(raf2);
