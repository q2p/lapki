import { event } from "@tauri-apps/api";
import { invoke } from "@tauri-apps/api/tauri";
import { log } from "console";
import e from "express";

type Config = {
  walls: Wall[];
  radio_points: RadioPoint[];
  radio_zones: RadioZone[];
}

type Wall = {
  a: Point2d;
  b: Point2d;
  damping: number;
}

type RadioPoint = {
  pos: Point2d;
  power: number;
}

type RadioZone = {
  points: Point2d[];
  r: number,
  g: number,
  b: number,
}

type Point2d = {
  x: number;
  y: number;
}

async function send_cmd(message: string): Promise<void> {
  invoke("cmd_do", { message })
}
async function get_config(): Promise<Config>{
  return await invoke("get_config") as Config
}

let config: Config = await get_config();

console.log(config)

// //50px = 1m
// const WIDTH = 60;
// const HEIGHT = 60;
// const KEY_L = 76;
// let key_l_clicked = 0;
// const KEY_S = 83;
// let key_s_clicked = 0;

// const grid = [
//   ['white', 'white'],
//   ['white', 'white']
// ];

// function drawGrid() {
//   // const startX = Math.floor((-stage.x() - stage.width()) / WIDTH) * WIDTH;
//   // const endX = Math.floor((-stage.x() + stage.width() * 2) / WIDTH) * WIDTH;

//   // const startY = Math.floor((-stage.y() - stage.height()) / HEIGHT) * HEIGHT;
//   // const endY = Math.floor((-stage.y() + stage.height() * 2) / HEIGHT) * HEIGHT;
//   let startX = 0;
//   let endX = 2500;
//   let startY = 0;
//   let endY = 2500;

//   for (var x = startX; x < endX; x += WIDTH) {
//     for (var y = startY; y < endY; y += HEIGHT) {

//       const indexX = ((x / WIDTH) + grid.length * WIDTH) % grid.length;
//       const indexY = ((y / HEIGHT) + grid[0].length * HEIGHT) % grid[0].length;

//       //maps from 0 to 3
//       const i = indexX * 2 + indexY;

//       layer.add(new Konva.Rect({
//         x,
//         y,
//         width: WIDTH,
//         height: HEIGHT,
//         fill: grid[indexX][indexY],
//         stroke: 'gray',
//         strokeWidth: 0.5
//       }));
//     }
//   }
// }

// function drawLines() {
//   for (let i = 0; i < config.walls.length; i++) {
//     let line = new Konva.Line({
//       x: 0,
//       y: 0,
//       points: [config.walls[i].a.x, config.walls[i].a.y, config.walls[i].b.x, config.walls[i].b.y,],
//       stroke: 'black',
//       tension: 1
//     });
//     // line.on("click", () => {
//     //   console.log("click");
//     // });
//     layer.add(line);
//   }
// }

// let scaleBy = 1.05;
// stage.on('wheel', (e) => {
//   // stop default scrolling
//   e.evt.preventDefault();

//   var oldScale = stage.scaleX();
//   var pointer = stage.getPointerPosition();

//   var mousePointTo = {
//     x: (pointer.x - stage.x()) / oldScale,
//     y: (pointer.y - stage.y()) / oldScale,
//   };

//   // how to scale? Zoom in? Or zoom out?
//   let direction = e.evt.deltaY > 0 ? 1 : -1;

//   // when we zoom on trackpad, e.evt.ctrlKey is true
//   // in that case lets revert direction
//   if (e.evt.ctrlKey) {
//     direction = -direction;
//   }

//   var newScale = direction > 0 ? oldScale * scaleBy : oldScale / scaleBy;

//   stage.scale({ x: newScale, y: newScale });

//   var newPos = {
//     x: pointer.x - mousePointTo.x * newScale,
//     y: pointer.y - mousePointTo.y * newScale,
//   };
//   stage.position(newPos);
// });

// // let container = stage.container();
// // // make container focusable
// // container.tabIndex = 1;
// // let pos = null;
// // let mem_pos = null;
// // container.addEventListener('keydown', function (e) {
// //   if (e.keyCode === KEY_L) {
// //     if (key_l_clicked == 1) {
// //       pos = stage.getRelativePointerPosition();
// //       let line = new Konva.Line({
// //         x: 0,
// //         y: 0,
// //         points: [mem_pos.x, mem_pos.y, pos.x, pos.y],
// //         stroke: 'black',
// //         tension: 1
// //       });
// //       layer.add(line);
// //     }
// //     mem_pos = stage.getRelativePointerPosition();
// //     key_l_clicked = 1;
// //   } else if (e.keyCode == KEY_S){
// //     key_l_clicked == 0;
// //     pos = mem_pos = null;
// //   } else {
// //     return;
// //   }
// //   e.preventDefault();
// // })

// stage.on('dragend', () => {
//   layer.destroyChildren();
//   drawGrid();
//   drawLines();
//   layer.draw();
// })

// stage.on('click', function () {
//   var pos = stage.getRelativePointerPosition();
//   console.log(pos)
// });

//////////////////////////
//////////////////////////
//////////////////////////

// rimg meters
const rimg_xmin = 5
const rimg_ymin = 12
const rimg_xmax = 60 - 0*55/600
const rimg_ymax = 52 - 0*40/436
const rimg_yc = (rimg_ymin + rimg_ymax)/2
const rimg_xc = (rimg_xmin + rimg_xmax)/2

let camX = rimg_xc;
let camY = rimg_yc;

let cursor_x_m = 0
let cursor_y_m = 0

const canvas = document.createElement("canvas")
document.body.appendChild(canvas)
const ctx = canvas.getContext("2d")!;

const image = new Image();
setInterval(function() {
  if (!image.complete) {
    image.src = image.src
  }
}, 200)

const wall_thickness = 8

let zoom_pow = 1
let zoom_target = 32
let zoom = zoom_target

window.addEventListener("wheel", function(e: WheelEvent) {
  zoom_pow = Math.max(-4, Math.min(6, zoom_pow - e.deltaY / 100))
  zoom_target = 32 * Math.pow(1.5, zoom_pow)
})

window.addEventListener("mousemove", function(e) {
  if (is_pressing) {
    camX = (canvas.width /2 - e.clientX) / zoom + anchorX
    camY = (canvas.height/2 - e.clientY) / zoom + anchorY
  }
  cursor_x_m = camX + (e.clientX - canvas.width  / 2) / zoom
  cursor_y_m = camY + (e.clientY - canvas.height / 2) / zoom
})

let anchorX = 0
let anchorY = 0
let is_pressing = false

window.addEventListener("mousedown", function(e) {
  is_pressing = true
  anchorX = camX + (e.clientX - canvas.width /2) / zoom
  anchorY = camY + (e.clientY - canvas.height/2) / zoom
})

window.addEventListener("mouseup", function() {
  is_pressing = false
})

let active_set = 0
window.addEventListener("keydown", function(e) {
  if (e.code === "KeyU") {
    send_cmd(prompt("enter cmd"))
  }
  if (e.code === "KeyS") {
    active_set = 1 - active_set
    elements = all_els[active_set]
    // image.src = active_set === 0 ? "../rimg3.png" : "../rimg4.png";
    camX = rimg_xc
    camY = rimg_yc
    zoom_target = Math.min(
      canvas.width/(rimg_xmax-rimg_xmin),
      canvas.height/(rimg_ymax-rimg_ymin),
    )
    zoom_pow = (Math.log(zoom_target / 32) / Math.log(1.5))
  }
})

function grid(offsetX: number, offsetY: number, grid_size_px: number, alpha: number) {
  ctx.strokeStyle = "#000";
  ctx.lineWidth = 1
  ctx.beginPath();
  for (let iy = -1; iy !== Math.floor(canvas.height / grid_size_px) + 2; iy++) {
    const ypos = Math.round((iy * grid_size_px) - (offsetY * zoom) % grid_size_px);
    ctx.moveTo(0, ypos);
    ctx.lineTo(canvas.width, ypos);
  }
  for (let ix = -1; ix !== Math.floor(canvas.width / grid_size_px) + 2; ix++) {
    const xpos = Math.round((ix * grid_size_px) - (offsetX * zoom) % grid_size_px);
    ctx.moveTo(xpos, 0);
    ctx.lineTo(xpos, canvas.height);
  }
  ctx.globalAlpha = alpha
  ctx.stroke();
  ctx.globalAlpha = 1.0
}

function legend(offsetX: number, offsetY: number) {
  let i = 0
  for (const el of elements) {
    let wh, text_size_base
    const past_zoom = Math.max(1, zoom / 100)
    wh = img_wh * past_zoom
    text_size_base = 15 * past_zoom

    const xpos = Math.round(zoom * (el.x - offsetX) - wh / 2)
    const ypos = Math.round(zoom * (el.y - offsetY) - wh / 2)

    points[i].style.left = `${Math.round(xpos)}px`
    points[i].style.top = `${Math.round(ypos)}px`
    points[i].style.width = `${wh}px`
    points[i].style.height = `${wh}px`

    tooltips[i].style.left = `${xpos}px`
    tooltips[i].style.top = `${ypos + wh * 1.2}px`
    tooltips[i].style.transform = "translateX(-50%)"
    tooltips[i].style.fontFamily = `sans-serif`
    tooltips[i].style.fontSize = `${text_size_base}px`
    tooltips[i].style.paddingLeft = `${text_size_base*6/15}px`;
    tooltips[i].style.paddingRight = `${text_size_base*6/15}px`;
    tooltips[i].style.paddingTop = `${text_size_base*2/15}px`;
    tooltips[i].style.paddingBottom = `${text_size_base*2/15}px`;

    i++
  }
}

function zc(i: number) {
  const z = config.radio_zones[i]
  return to_rgb(z)
}

function to_rgb(z: RadioZone) {
  return "#" + (0x1000000 + z.r*256*256 + z.g*256 + z.b).toString(16).slice(-6)
}

const all_els = [[
  {label: 'Red Power: 109mv',   x: config.radio_points[0].pos.x, y: config.radio_points[0].pos.y, color: zc(0)},
  {label: 'Green Power: 212mv', x: config.radio_points[1].pos.x, y: config.radio_points[1].pos.y, color: zc(1)},
  {label: 'Blue Power: 184mv',  x: config.radio_points[2].pos.x, y: config.radio_points[2].pos.y, color: zc(2)},
  {label: 'Min SINR Red: 15db',   x: 42.9, y: 36.1, color: zc(0)},
  {label: 'Min SINR Green: 6db', x: 36.1, y: 22.1, color: zc(1)},
  {label: 'Min SINR Blue: -2db',  x: 42.5, y: 21.9,  color: zc(2)},
],
[
  {label: 'Max Red',   x: config.radio_points[0].pos.x, y: config.radio_points[0].pos.y, color: zc(0)},
  {label: 'Max Green', x: config.radio_points[1].pos.x, y: config.radio_points[1].pos.y, color: zc(1)},
  {label: 'Max Blue',  x: config.radio_points[2].pos.x, y: config.radio_points[2].pos.y, color: zc(2)},
  {label: 'Min Red',   x: 42.9, y: 36.1, color: zc(0)},
  {label: 'Min Green', x: 36.1, y: 22.1, color: zc(1)},
  {label: 'Min Blue',  x: 41.9, y: 21.9,  color: zc(2)},
]];

all_els[0] = []
all_els[1] = []

let elements = all_els[0]

// let img = document.getElementById('rimg');
const img_wh = 12;

for (const el of elements) {
  const img = document.createElement('div');
  img.className = "legend_point";
  img.style.position = "absolute";
  img.style.border = "2px solid black";
  img.style.borderRadius = "100%";
  img.style.width = `${img_wh}px`;
  img.style.height = `${img_wh}px`;
  img.style.backgroundColor = el.color;

  let box = document.createElement('span');
  box.className = "legend_tooltip";
  box.textContent = el.label;
  box.style.position = "absolute";
  box.style.background = "rgba(33,33,33,0.7)";
  box.style.color = "white";
  box.style.borderRadius = "5px";

  document.body.appendChild(box);
  document.body.appendChild(img);
};

let points = document.getElementsByClassName("legend_point") as HTMLCollectionOf<HTMLDivElement>
let tooltips = document.getElementsByClassName("legend_tooltip") as HTMLCollectionOf<HTMLSpanElement>

function normalize(p: Point2d) {
  const len = Math.sqrt(p.x * p.x + p.y * p.y)
  return <Point2d> { x: p.x / len, y: p.y / len }
}

function raf() {
  requestAnimationFrame(raf)

  // const t = new Date().getTime()

  zoom = zoom_target * 0.1 + zoom * 0.9
  if (Math.abs(zoom - zoom_target) < 0.001) {
    zoom = zoom_target
  }

  // let camX = rimg_xc+16*Math.sin(t*0.0001);
  // let camY = rimg_yc+16*Math.cos(t*0.0001);
  // let zoom = 24+8*Math.cos(1253.0+t*0.0001);
  // zoom = 32
  let offsetX = camX - canvas.width / (2*zoom);
  let offsetY = camY - canvas.height / (2*zoom);

  ctx.fillStyle = "#fff"
  ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height)

  if (image.complete) {
    const dx = Math.round((rimg_xmin-offsetX)*zoom)
    const dy = Math.round((rimg_ymin-offsetY)*zoom)
    const rx = Math.round((rimg_xmax-rimg_xmin)*zoom)
    const ry = Math.round((rimg_ymax-rimg_ymin)*zoom)
    ctx.drawImage(image, dx, dy, rx, ry)
    ctx.fillRect(0, 0, dx, canvas.height)
    ctx.fillRect(dx+rx, 0, canvas.width, canvas.height)
    ctx.fillRect(dx, 0, rx, dy)
    ctx.fillRect(dx, dy+ry, rx, canvas.height)
  } else {
    ctx.fillRect(0, 0, canvas.width, canvas.height)
  }

  if (zoom >= 128) {
    grid(offsetX, offsetY, zoom/10, 0.5)
  }
  if (zoom >= 8) {
    grid(offsetX, offsetY, zoom, 0.7)
  }

  //////////////////////////////

  legend(offsetX, offsetY)
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

  for (const zone of config.radio_zones.reverse()) {
    const path = [];
    for (const p of zone.points) {
      path.push(<Point2d> { x: p.x, y: p.y })
    }
    for (let i = 0; i !== path.length; i++) {
      const a: Point2d = path[i]
      const b: Point2d = path[(i + 1) % path.length]
      const dx = b.x - a.x;
      const dy = b.y - a.y;
      const normal = normalize(<Point2d> { x: dy, y: -dx })
      // const normal = <Point2d> {x: 0, y: 0}
      a.x += normal.x * 0.5;
      b.x += normal.x * 0.5;
      a.y += normal.y * 0.5;
      b.y += normal.y * 0.5;
    }

    ctx.globalAlpha = 0.5;
    ctx.fillStyle = to_rgb(zone);
    ctx.beginPath();
    for (let i = 0; i !== path.length; i++) {
      const xpos = Math.round(zoom * (path[i].x - offsetX))
      const ypos = Math.round(zoom * (path[i].y - offsetY))
      if (i === 0) {
        ctx.moveTo(xpos, ypos);
      } else {
        ctx.lineTo(xpos, ypos);
      }
    }
    ctx.closePath();
    ctx.fill();
    ctx.globalAlpha = 1.0;
  }


  ///////////////////////

  ctx.lineWidth = wall_thickness
  ctx.strokeStyle = "#000";
  ctx.fillStyle = "#000"
  ctx.beginPath();
  for (const w of config.walls) {
    const xmin = Math.round((Math.min(w.a.x, w.b.x)-offsetX)*zoom)
    const xmax = Math.round((Math.max(w.a.x, w.b.x)-offsetX)*zoom)
    const ymin = Math.round((Math.min(w.a.y, w.b.y)-offsetY)*zoom)
    const ymax = Math.round((Math.max(w.a.y, w.b.y)-offsetY)*zoom)
    ctx.fillRect(xmin - wall_thickness/2, ymin - wall_thickness/2, xmax-xmin + wall_thickness, ymax-ymin + wall_thickness)

    // Раскомментить для угловатых стен.
    // w.a.x = Math.round(w.a.x)
    // w.a.y = Math.round(w.a.y)
    // w.b.x = Math.round(w.b.x)
    // w.b.y = Math.round(w.b.y)
    // ctx.moveTo((w.a.x-offsetX)*zoom, (w.a.y-offsetY)*zoom);
    // ctx.lineTo((w.b.x-offsetX)*zoom, (w.b.y-offsetY)*zoom);
  }
  ctx.stroke();

  const scales = [1, 2, 5]
  for (let i = -10; i !== 32; i++) {
    const decs = Math.floor(i / 3)
    const scale = scales[((i % 3) + 3) % 3]
    const meters = Math.pow(10, decs) * scale
    if (meters * zoom > 64) {
      ctx.lineWidth = 1
      ctx.strokeStyle = "#000";
      ctx.fillStyle = "#000"

      ctx.font = "bold 22px sans-serif"
      const width = Math.round(meters * zoom)
      ctx.textAlign = "center"
      ctx.fillText(`${meters}m`, 32 + width/2, canvas.height-50)
      ctx.fillRect(32, canvas.height-34, width, 2)
      ctx.fillRect(32      , canvas.height-37, 2, 8)
      ctx.fillRect(32+width, canvas.height-37, 2, 8)
      break
    }
  }
}

function resize() {
  console.log("onresize")
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
}

window.addEventListener("resize", resize)
document.addEventListener("resize", resize)
resize()
requestAnimationFrame(raf)
