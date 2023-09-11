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
}

type Point2d = {
  x: number;
  y: number;
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
const xmin = 5.2
const ymin = 12.4
const xmax = 59.6
const ymax = 52.4
const rimg_yc = (ymin + ymax)/2
const rimg_xc = (xmin + xmax)/2

let camX = 0;
let camY = 0;

const canvas = document.createElement("canvas")
document.body.appendChild(canvas)
const ctx = canvas.getContext("2d")!;

let image_loaded = false
const image = new Image();
image.src = "../rimg3.png";
image.onload = function () { image_loaded = true }

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

function grid(offsetX: number, offsetY: number, grid_size_px: number, color: string) {
  ctx.strokeStyle = color;
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
  ctx.stroke();
}

function legend(offsetX: number, offsetY: number) {
  let i = 0
  const dx = Math.round((xmin-offsetX)*zoom)
  const dy = Math.round((ymin-offsetY)*zoom)
  for (const el of elements) {
    let wh
    let text_size_base = 15
    if (zoom >= 128) {
      wh = Math.round(img_wh*zoom/100)
      text_size_base = Math.round(text_size_base*zoom/100)
    } else {
      wh = img_wh    
    }

    let xpos = (Math.round(el.x * zoom - wh / 2)) + dx
    let ypos = (Math.round(el.y * zoom - wh / 2)) + dy
    
    points[i].style.left = `${Math.round(xpos)}px`
    points[i].style.top = `${Math.round(ypos)}px`
    points[i].style.width = `${wh}px`
    points[i].style.height = `${wh}px`

    tooltips[i].style.left = `${xpos + wh}px`
    tooltips[i].style.top = `${ypos}px`
    tooltips[i].style.fontSize = `${text_size_base}px`

    i++
  }
}

const elements = [
  {label: 'max red', x: 10, y: 5, color: "red"},
  {label: 'min red', x: 5, y: 5, color: "red"},
  {label: 'max blue', x: 12, y: 7, color: "blue"},
  {label: 'min blue', x: 7, y: 7, color: "blue"},
  {label: 'max green', x: 15, y: 10, color: "green"},
  {label: 'min green', x: 10, y: 10, color: "green"},
];

// let img = document.getElementById('rimg');
let img_wh = 8;

for (const el of elements) {
  const img = document.createElement('div');
  img.className = "legend_point";
  img.style.position = "absolute";
  img.style.border = "2px solid black";
  img.style.borderRadius = "100%";
  img.style.width = img_wh.toString() + "px";
  img.style.height = img_wh.toString() + "px";
  img.style.backgroundColor = el.color;

  let box = document.createElement('span');
  box.className = "legend_tooltip";
  box.textContent = el.label;
  box.style.position = "absolute";
  box.style.opacity = "0.7";
  box.style.background = "#333";
  box.style.color = "white";
  box.style.borderRadius = "5px";
  box.style.paddingLeft = "6px";
  box.style.paddingRight = "6px";
  box.style.paddingTop = "2px";
  box.style.paddingBottom = "2px";

  document.body.appendChild(box);
  document.body.appendChild(img);
};

let points = document.getElementsByClassName("legend_point") as HTMLCollectionOf<HTMLDivElement>
let tooltips = document.getElementsByClassName("legend_tooltip") as HTMLCollectionOf<HTMLSpanElement>

function raf() {
  requestAnimationFrame(raf)

  // const t = new Date().getTime()

  zoom = zoom_target * 0.1 + zoom * 0.9

  // let camX = rimg_xc+16*Math.sin(t*0.0001);
  // let camY = rimg_yc+16*Math.cos(t*0.0001);
  // let zoom = 24+8*Math.cos(1253.0+t*0.0001);
  // zoom = 32
  let offsetX = camX - canvas.width / (2*zoom);
  let offsetY = camY - canvas.height / (2*zoom);

  ctx.fillStyle = "#fff"
  ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height)

  if (zoom >= 128) {
    grid(offsetX, offsetY, zoom/10, "#999")
  }
  if (zoom >= 8) {
    grid(offsetX, offsetY, zoom, "#000")
  }

  ctx.globalAlpha = 0.5;
  if (image_loaded) {
    const dx = Math.round((xmin-offsetX)*zoom)
    const dy = Math.round((ymin-offsetY)*zoom)
    const rx = Math.round((xmax-xmin)*zoom)
    const ry = Math.round((ymax-ymin)*zoom)
    ctx.drawImage(image, dx, dy, rx, ry)
    ctx.fillRect(0, 0, dx, canvas.height)
    ctx.fillRect(dx+rx, 0, canvas.width, canvas.height)
    ctx.fillRect(dx, 0, rx, dy)
    ctx.fillRect(dx, dy+ry, rx, canvas.height)
  } else {
    ctx.fillRect(0, 0, canvas.width, canvas.height)
  }
  ctx.globalAlpha = 1;

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

  ctx.lineWidth = wall_thickness
  ctx.strokeStyle = "#000";
  ctx.fillStyle = "#000"
  ctx.beginPath();
  for (const w of config.walls) {
    w.a.x = Math.round(w.a.x)
    w.a.y = Math.round(w.a.y)
    w.b.x = Math.round(w.b.x)
    w.b.y = Math.round(w.b.y)
    const xmin = Math.round((Math.min(w.a.x, w.b.x)-offsetX)*zoom)
    const xmax = Math.round((Math.max(w.a.x, w.b.x)-offsetX)*zoom)
    const ymin = Math.round((Math.min(w.a.y, w.b.y)-offsetY)*zoom)
    const ymax = Math.round((Math.max(w.a.y, w.b.y)-offsetY)*zoom)
    ctx.fillRect(xmin - wall_thickness/2, ymin - wall_thickness/2, xmax-xmin + wall_thickness, ymax-ymin + wall_thickness)
    ctx.moveTo((w.a.x-offsetX)*zoom, (w.a.y-offsetY)*zoom);
    ctx.lineTo((w.b.x-offsetX)*zoom, (w.b.y-offsetY)*zoom);
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

function push_aside2() {
  let spans = document.querySelectorAll('span');
  let prev: HTMLSpanElement | null = null;
  let prev_rect: DOMRect | null = null;

    spans.forEach(function(el) {
      if (prev === null && prev_rect === null) {
        prev = el;
        prev_rect = el.getBoundingClientRect();
        return;
      }

      let rect = el.getBoundingClientRect();
      if(rect.bottom > prev_rect.top
        && rect.right > prev_rect.left
        && rect.top < prev_rect.bottom
        && rect.left < prev_rect.right) {
          let dy = Math.abs(rect.top - prev_rect.bottom);
          let dx = Math.abs(rect.left - prev_rect.right);
          prev.style.top = (prev.offsetTop - dy).toString() + "px";

          el.style.top = (el.offsetTop + img_wh).toString() + "px";
          el.style.left = (el.offsetLeft - el.clientWidth - img_wh).toString() + "px";
          prev.style.margin = "3px";
          el.style.margin = "3px";
          // prev.style.left = (prev.offsetLeft + (dx / 2)).toString() + "px";
        }
      prev = el;
      prev_rect = el.getBoundingClientRect();
    });
  }
