import { emit, listen } from "@tauri-apps/api/event"
import { invoke } from "@tauri-apps/api/tauri"
import { open, save as save_dialog } from "@tauri-apps/api/dialog"
import { appWindow } from "@tauri-apps/api/window"
import { dialog } from "@tauri-apps/api"
import { TauriEvent } from "@tauri-apps/api/event"
import { ActiveBest, AppConfig, AppState, Config, DrawingState, Point2d, StoredBest, Wall, BoundingBoxes, Stack, wall_eq, MATERIALS } from './types';
import { get_active_best, get_app_config, get_bb, get_config, should_play, write_app_config } from "./api"
import { quit, save } from "./actions"
import { registerGlobalListeners } from "./listeners"
import { registerGlobalShortcuts } from "./shortcuts"

const style = (node, styles) =>
  Object.keys(styles).forEach((key) => (node.style[key] = styles[key]))

// let app_config: AppConfig = await get_app_config()
// let config: Config
// let config_path: string
// let changes: boolean = false
const app_state = <AppState> {
  app_config: await get_app_config(),
  changes: false,
}

// let drawing: boolean = false;
// let mevent: MouseEvent;
// let drawing_last_point: Point2d | null = null;
// let drawing_point: Point2d | null = null;
// let added_walls: Wall[] = [];
const drawing_state = <DrawingState>{
  drawing: false,
  drawing_last_point: null,
  drawing_point: null,
}
drawing_state.added_walls = []

registerGlobalListeners(app_state, drawing_state)
registerGlobalShortcuts(app_state, drawing_state)

let camX = 0
let camY = 0

let cursor_x_m = 0
let cursor_y_m = 0
let cursor_x_p = 0
let cursor_y_p = 0

const canvas = document.createElement("canvas")
document.body.appendChild(canvas)
const ctx = canvas.getContext("2d")!

const image = new Image()
setInterval(function() {
  if (!image.complete) {
    image.src = image.src
  }
}, 200)

const wall_thickness = 6 // 8

let zoom_pow = 1
let zoom_target = 32
let zoom = zoom_target

let radius = 10

window.addEventListener("wheel", function(e: WheelEvent) {
  zoom_pow = Math.max(-4, Math.min(6, zoom_pow - e.deltaY / 100))
  zoom_target = 32 * Math.pow(1.5, zoom_pow)
})

window.addEventListener("mousemove", function(e) {
  cursor_x_p = e.clientX
  cursor_y_p = e.clientY
  if (is_pressing) {
    camX = anchor_x_m - (cursor_x_p - canvas.width / 2) / zoom
    camY = anchor_y_m - (cursor_y_p - canvas.height / 2) / zoom
  }
  cursor_x_m = camX + (cursor_x_p - canvas.width / 2) / zoom
  cursor_y_m = camY + (cursor_y_p - canvas.height / 2) / zoom

  drawing_state.mevent = e
})

let anchor_x_m = 0
let anchor_y_m = 0
let is_pressing = false

let drawing_pressed = false

let stack_points = new Stack()

function in_line(a: Point2d, b: Point2d, c: Point2d) {
  const offsetX = camX - canvas.width / (2 * zoom)
  const offsetY = camY - canvas.height / (2 * zoom)
  const xmin = Math.round((Math.min(a.x, b.x) - offsetX) * zoom)
  const xmax = Math.round((Math.max(a.x, b.x) - offsetX) * zoom)
  const ymin = Math.round((Math.min(a.y, b.y) - offsetY) * zoom)
  const ymax = Math.round((Math.max(a.y, b.y) - offsetY) * zoom)
  const dxL = xmax - xmin, dyL = ymax - ymin  // line: vector from (x1,y1) to (x2,y2)
  const dxP = c.x - xmin, dyP = c.y - ymin  // point: vector from (x1,y1) to (xp,yp)
  const squareLen = dxL * dxL + dyL * dyL;  // squared length of line
  const dotProd   = dxP * dxL + dyP * dyL;  // squared distance of point from (x1,y1) along line
  const crossProd = dyP * dxL - dxP * dyL;  // area of parallelogram defined by line and point

  // perpendicular distance of point from line
  const distance = Math.abs(crossProd) / Math.sqrt(squareLen);

  return (distance <= wall_thickness && dotProd >= 0 && dotProd <= squareLen);
}

function mul_vec(v: Point2d, n: number): Point2d {
  return {
    x: v.x * n,
    y: v.y * n,
  }
}
function add_vec(a: Point2d, b: Point2d): Point2d {
  return {
    x: a.x + b.x,
    y: a.y + b.y,
  }
}
function sub_vec(a: Point2d, b: Point2d): Point2d {
  return {
    x: a.x - b.x,
    y: a.y - b.y,
  }
}
function dot_prod(v1: Point2d, v2: Point2d): number {
  return v1.x * v2.x + v1.y * v2.y
}
function cross_prod(v1: Point2d, v2: Point2d): number {
  return v1.x * v2.y - v1.y * v2.x;
}

let selected: Wall[] = []
const elprops = document.getElementById("el_props") as HTMLDivElement
const elprops_title = document.getElementById("title") as HTMLHeadingElement
const elprops_container = document.getElementById("container") as HTMLDivElement
elprops.style.display = "none"

function show_elprops(wall: Wall) {
  elprops.style.display = ""
  elprops_title.textContent = "Wall"
  const btn = document.createElement("button")
  btn.textContent = "Delete"
  btn.onclick = function() {
    let idx = app_state.config.walls
      .findIndex(w => wall_eq(wall, w))
    if (idx > -1) {
      app_state.config.walls.splice(idx, 1)
    }
    idx = drawing_state.added_walls
      .findIndex(w => wall_eq(wall, w))
    if (idx > -1) {
      drawing_state.added_walls.splice(idx, 1)
    }
    selected = []
  }
  elprops_container.appendChild(btn)
  const x = document.createElement("div")
  const label = document.createElement("label")
  label.textContent = "Material"
  const select = document.createElement("select")
  for (const material of MATERIALS) {
    const option = document.createElement("option")
    option.value = material[1].toString()
    option.textContent = material[0]
    if (wall.damping === material[1]) option.selected = true
    select.appendChild(option)
  }
  select.onchange = function () {
    wall.damping = parseInt(select.value)
  }
  x.appendChild(label)
  x.appendChild(select)
  elprops_container.appendChild(x)
}

function hide_elprops() {
  elprops.style.display = "none"
  while (elprops_container.firstChild) {
    elprops_container.removeChild(elprops_container.firstChild);
  }
}

canvas.addEventListener("click", function(e) {
  hide_elprops()
  selected = []
  cursor_x_p = e.clientX
  cursor_y_p = e.clientY
  cursor_x_m = camX + (cursor_x_p - canvas.width / 2) / zoom
  cursor_y_m = camY + (cursor_y_p - canvas.height / 2) / zoom

  let next = false
  for (let wall of app_state.config.walls) {
    if (in_line(wall.a, wall.b, <Point2d>{x: cursor_x_p, y: cursor_y_p})) {
      selected.push(wall)
      next = true
      show_elprops(wall)
      break
    }
  }
  for (let wall of drawing_state.added_walls) {
    if (next) {
      break
    }
    if (in_line(wall.a, wall.b, <Point2d>{x: cursor_x_p, y: cursor_y_p})) {
      selected.push(wall)
      show_elprops(wall)
    }
  }
})

window.addEventListener("mousedown", function(e) {
  cursor_x_p = e.clientX
  cursor_y_p = e.clientY
  cursor_x_m = camX + (cursor_x_p - canvas.width / 2) / zoom
  cursor_y_m = camY + (cursor_y_p - canvas.height / 2) / zoom
  if (drawing_state.drawing) {
    // drawing_pressed = true
    const point = {
      x: Math.round(cursor_x_m * 10) / 10,
      y: Math.round(cursor_y_m * 10) / 10,
    }
    stack_points.add(point)
    if (stack_points.size() >=2) {
      let first = stack_points.remove()
      let second = stack_points.remove()
      stack_points.clear()
      let wall: Wall = {
        a: {
          x: first.x,
          y: first.y
        },
        b: {
          x: second.x,
          y: second.y
        },
        damping: 2.0
      }
      drawing_state.added_walls.push(wall)
      app_state.changes = true
      if (app_state.config_path) {
        appWindow.setTitle("[" + app_state.config_path.split("\\").pop() + "*] – " + "5G Planner ")
      } else {
        appWindow.setTitle("[Untitled" + "*" + "]" + "– 5G Planner")
      }
    }
  } else {
    is_pressing = true
    anchor_x_m = cursor_x_m
    anchor_y_m = cursor_y_m
  }
})

window.addEventListener("mouseup", function(e) {
  cursor_x_p = e.clientX
  cursor_y_p = e.clientY
  cursor_x_m = camX + (cursor_x_p - canvas.width / 2) / zoom
  cursor_y_m = camY + (cursor_y_p - canvas.height / 2) / zoom
  if (drawing_state.drawing) {
    // if (
    //   drawing_state.drawing_last_point &&
    //   drawing_state.drawing_point &&
    //   Math.abs(cursor_x_m - drawing_state.drawing_last_point.x) <= 0.9 &&
    //   Math.abs(cursor_y_m - drawing_state.drawing_last_point.y) <= 0.9
    // ) {
    //   drawing_pressed = false
    //   drawing_state.drawing_last_point = null
    // } else {
    //   drawing_pressed = false
    //   drawing_state.drawing_point = {
    //     x: cursor_x_m,
    //     y: cursor_y_m,
    //   }
    //   const wall: Wall = {
    //     a: drawing_state.drawing_last_point!,
    //     b: drawing_state.drawing_point!,
    //     damping: 500, // fix me
    //   }
    //   drawing_state.added_walls.push(wall)
    //   app_state.changes = true
    //   if (app_state.config_path) {
    //     appWindow.setTitle("[" + app_state.config_path.split("\\").pop() + "*] – " + "5G Planner ")
    //   } else {
    //     appWindow.setTitle("[Untitled" + "*" + "]" + "– 5G Planner")
    //   }
    // }
  } else {
    is_pressing = false
  }
})

export function center_view() {
  let min_x = Infinity
  let min_y = Infinity
  let max_x = -Infinity
  let max_y = -Infinity
  for (const wall of app_state.config.walls) {
    for (const i of [wall.a, wall.b]) {
      min_x = Math.min(min_x, i.x)
      min_y = Math.min(min_y, i.y)
      max_x = Math.max(max_x, i.x)
      max_y = Math.max(max_y, i.y)
    }
  }
  camX = (min_x + max_x) / 2
  camY = (min_y + max_y) / 2
  zoom_target = Math.min(
    canvas.width / (max_x - min_x),
    canvas.height / (max_y - min_y),
  ) * 0.95
  zoom_pow = Math.log(zoom_target / 32) / Math.log(1.5)
  get_active_best().then(update_active_els)
}

let BB: BoundingBoxes = {
  min: {x: 0, y: 0},
  max: {x: 0, y: 0},
  res: [0, 0],
  wh: {x: 0, y: 0}
}

let rimg_id = 0;
window.addEventListener("keydown", async function(e) {
  if (e.code === "KeyS") {
    rimg_id = (rimg_id + 1) % 3;
    image.src = `../rimg${1 + rimg_id}.png?${Math.floor(Math.random() * 100000)}`
    center_view()
    BB = await get_bb()
  }
})

function mk_tooltip(x: number, y: number, color: string, ...text: string[]) {
  const img = document.createElement("div")
  img.className = "legend_point"
  img.style.position = "absolute"
  img.style.border = "2px solid black"
  img.style.borderRadius = "100%"
  img.style.width = `${img_wh}px`
  img.style.height = `${img_wh}px`
  img.style.backgroundColor = color

  const box = document.createElement("div")
  box.className = "legend_tooltip"
  for (const t of text) {
    box.appendChild(document.createTextNode(t))
    box.appendChild(document.createElement("br"))
  }
  box.style.position = "absolute"
  box.style.background = "rgba(33,33,33,0.7)"
  box.style.color = "white"
  box.style.whiteSpace = "nowrap"
  box.style.borderRadius = "5px"

  document.body.appendChild(box)
  document.body.appendChild(img)

  elements.push(new StoredBest(x, y, box, img))
}

function update_active_els(a: ActiveBest[]) {
  for (const el of elements) {
    el.circle.remove()
    el.tooltip.remove()
  }
  for (const el of a) {
    const color = ac(el.r, el.g, el.b)
    mk_tooltip(
      el.point_x,
      el.point_y,
      color,
      `Pow: ${el.point_pow_mw.toPrecision(4)}mw.`,
      `Median SINR: ${el.median_sinr_dbm.toPrecision(4)}db.`,
    )
  }
}

function grid(
  offsetX: number,
  offsetY: number,
  grid_size_px: number,
  alpha: number,
) {
  ctx.strokeStyle = "#000"
  ctx.lineWidth = 1
  ctx.beginPath()
  for (let iy = -1; iy !== Math.floor(canvas.height / grid_size_px) + 2; iy++) {
    const ypos = Math.round(
      iy * grid_size_px - ((offsetY * zoom) % grid_size_px),
    )
    ctx.moveTo(0, ypos)
    ctx.lineTo(canvas.width, ypos)
  }
  for (let ix = -1; ix !== Math.floor(canvas.width / grid_size_px) + 2; ix++) {
    const xpos = Math.round(
      ix * grid_size_px - ((offsetX * zoom) % grid_size_px),
    )
    ctx.moveTo(xpos, 0)
    ctx.lineTo(xpos, canvas.height)
  }
  ctx.globalAlpha = alpha
  ctx.stroke()
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

    el.circle.style.left = `${Math.round(xpos)}px`
    el.circle.style.top = `${Math.round(ypos)}px`
    el.circle.style.width = `${wh}px`
    el.circle.style.height = `${wh}px`

    el.tooltip.style.left = `${xpos}px`
    el.tooltip.style.top = `${ypos + wh * 1.2}px`
    el.tooltip.style.transform = "translateX(-50%)"
    el.tooltip.style.fontFamily = "sans-serif"
    el.tooltip.style.fontSize = `${text_size_base}px`
    el.tooltip.style.paddingLeft = `${(text_size_base * 6) / 15}px`
    el.tooltip.style.paddingRight = `${(text_size_base * 6) / 15}px`
    el.tooltip.style.paddingTop = `${(text_size_base * 2) / 15}px`
    el.tooltip.style.paddingBottom = `${(text_size_base * 2) / 15}px`

    i++
  }
}

function zc(i: number) {
  const z = app_state.config.radio_zones[i]
  return ac(z.r, z.g, z.b)
}

function ac(r: number, g: number, b: number) {
  return "#" + (0x1000000 + r * 256 * 256 + g * 256 + b).toString(16).slice(-6)
}

const elements: StoredBest[] = []

// let img = document.getElementById('rimg');
const img_wh = 12

function raf() {
  requestAnimationFrame(raf)

  ctx.imageSmoothingEnabled = true
  ctx.imageSmoothingQuality = "high"

  // const t = new Date().getTime()

  zoom = zoom_target * 0.1 + zoom * 0.9
  if (Math.abs(zoom - zoom_target) < 0.001) {
    zoom = zoom_target
  }

  // TODO: Убрать?
  if (is_pressing) {
    camX = anchor_x_m - (cursor_x_p - canvas.width / 2) / zoom
    camY = anchor_y_m - (cursor_y_p - canvas.height / 2) / zoom
  }

  // let camX = rimg_xc+16*Math.sin(t*0.0001);
  // let camY = rimg_yc+16*Math.cos(t*0.0001);
  // let zoom = 24+8*Math.cos(1253.0+t*0.0001);
  // zoom = 32
  const offsetX = camX - canvas.width / (2 * zoom)
  const offsetY = camY - canvas.height / (2 * zoom)

  ctx.fillStyle = "#fff"
  ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height)

  if (image.complete) {
    const dx = Math.round((BB.min.x - offsetX) * zoom)
    const dy = Math.round((BB.min.y - offsetY) * zoom)
    const rx = Math.round(BB.wh.x * zoom)
    const ry = Math.round(BB.wh.y * zoom)
    ctx.drawImage(image, dx, dy, rx, ry)
    ctx.fillRect(0, 0, dx, canvas.height)
    ctx.fillRect(dx + rx, 0, canvas.width, canvas.height)
    ctx.fillRect(dx, 0, rx, dy)
    ctx.fillRect(dx, dy + ry, rx, canvas.height)
  } else {
    ctx.fillRect(0, 0, canvas.width, canvas.height)
  }

  if (zoom >= 128) {
    grid(offsetX, offsetY, zoom / 10, 0.5)
  }
  if (zoom >= 8) {
    grid(offsetX, offsetY, zoom, 0.7)
  }

  /// ///////////////////////////

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

  /// //////////////////////////////////////

  // DRAW RADIOZONES

  if (!running) {
    for (const zone of app_state.config.radio_zones) {
      const zone2: Point2d[] = []
      for (const p of zone.points) {
        zone2.push({ x: p.x, y: p.y })
      }
      const shift = 0.05
      for (let i = 0; i !== zone2.length; i++) {
        const a = zone2[i]
        const b = zone2[(i + 1) % zone2.length]
        const n = normalize({
          x: b.y - a.y,
          y: a.x - b.x,
        })
        a.x += n.x * shift
        a.y += n.y * shift
        b.x += n.x * shift
        b.y += n.y * shift
      }
      ctx.beginPath()
      ctx.fillStyle = `rgba(${zone.r}, ${zone.g}, ${zone.b}, 0.5)`
      ctx.moveTo((zone2[0].x-offsetX)*zoom, (zone2[0].y -offsetY)*zoom)
      for (let i = 1; i < zone.points.length; i++) {
        ctx.lineTo((zone2[i].x-offsetX)*zoom, (zone2[i].y -offsetY)*zoom)
      }
      ctx.closePath()
      ctx.fill();
    }
  }


  // TODO: CHECK IF UNDEFINED OR NUL
  ctx.lineWidth = wall_thickness
  ctx.strokeStyle = "#000"
  ctx.fillStyle = "#000"
  ctx.lineJoin = "round"
  ctx.lineCap = "round"
  ctx.beginPath()
  for (const w of app_state.config.walls) {
    const xmin = Math.round((Math.min(w.a.x, w.b.x) - offsetX) * zoom)
    const xmax = Math.round((Math.max(w.a.x, w.b.x) - offsetX) * zoom)
    const ymin = Math.round((Math.min(w.a.y, w.b.y) - offsetY) * zoom)
    const ymax = Math.round((Math.max(w.a.y, w.b.y) - offsetY) * zoom)
    ctx.moveTo(xmin, ymin)
    ctx.lineTo(xmax, ymax)
    ctx.stroke()
  }

  if (drawing_state.added_walls.length > 0) {
    ctx.lineWidth = wall_thickness
    ctx.strokeStyle = "#000"
    for (const w of drawing_state.added_walls) {
      const xmin = Math.round((Math.min(w.a.x, w.b.x) - offsetX) * zoom)
      const xmax = Math.round((Math.max(w.a.x, w.b.x) - offsetX) * zoom)
      const ymin = Math.round((Math.min(w.a.y, w.b.y) - offsetY) * zoom)
      const ymax = Math.round((Math.max(w.a.y, w.b.y) - offsetY) * zoom)
      // ctx.fillRect(
      //   xmin - wall_thickness / 2,
      //   ymin - wall_thickness / 2,
      //   xmax - xmin + wall_thickness,
      //   ymax - ymin + wall_thickness,
      // )
      ctx.moveTo(xmin, ymin)
      ctx.lineTo(xmax, ymax)
      ctx.stroke()
    }
  }

  if (selected.length > 0) {
    for (const w of selected) {
      ctx.beginPath()
      const xmin = Math.round((Math.min(w.a.x, w.b.x) - offsetX) * zoom)
      const xmax = Math.round((Math.max(w.a.x, w.b.x) - offsetX) * zoom)
      const ymin = Math.round((Math.min(w.a.y, w.b.y) - offsetY) * zoom)
      const ymax = Math.round((Math.max(w.a.y, w.b.y) - offsetY) * zoom)
      ctx.fillStyle = "blue"
      ctx.fillRect(
        xmin - wall_thickness / 2,
        ymin - wall_thickness / 2 ,
        xmax - xmin + wall_thickness,
        ymax - ymin + wall_thickness
      )
      ctx.fillStyle = "rgb(0, 255, 255)"
      ctx.arc(xmin, ymin, radius, 0, 2 * Math.PI, false)
      ctx.arc(xmax, ymax, radius, 0, 2 * Math.PI, false)
      ctx.fill()
    }
  }

  // if (drawing_state.drawing) {
  //   // Рисуем точку на курсоре мыши
  //   ctx.strokeStyle = "#000"
  //   ctx.fillStyle = "#000"
  //   ctx.beginPath()
  //   ctx.fillRect(
  //     drawing_state.mevent.clientX - wall_thickness / 2,
  //     drawing_state.mevent.clientY - wall_thickness / 2,
  //     wall_thickness,
  //     wall_thickness,
  //   )
  //   ctx.stroke()

  //   if (drawing_state.drawing_last_point) {
  //     const x = Math.round((drawing_state.drawing_last_point.x - offsetX) * zoom)
  //     const y = Math.round((drawing_state.drawing_last_point.y - offsetY) * zoom)
  //     ctx.strokeStyle = "#000"
  //     ctx.fillStyle = "#000"
  //     ctx.beginPath()
  //     ctx.fillRect(
  //       x - wall_thickness / 2,
  //       y - wall_thickness / 2,
  //       wall_thickness,
  //       wall_thickness,
  //     )
  //     ctx.stroke()
  //     if (drawing_state.drawing_point) {
  //       const x = Math.round((drawing_state.drawing_point.x - offsetX) * zoom)
  //       const y = Math.round((drawing_state.drawing_point.y - offsetY) * zoom)
  //       ctx.strokeStyle = "#000"
  //       ctx.fillStyle = "#000"
  //       ctx.beginPath()
  //       ctx.fillRect(
  //         x - wall_thickness / 2,
  //         y - wall_thickness / 2,
  //         wall_thickness,
  //         wall_thickness,
  //       )
  //       ctx.stroke()
  //     }
  //     if (drawing_pressed) {
  //       ctx.lineWidth = wall_thickness / 4
  //       ctx.strokeStyle = "#000"
  //       ctx.fillStyle = "#000"
  //       ctx.beginPath()
  //       ctx.moveTo(
  //         (drawing_state.drawing_last_point.x - offsetX) * zoom,
  //         (drawing_state.drawing_last_point.y - offsetY) * zoom,
  //       )
  //       ctx.lineTo(drawing_state.mevent.offsetX, drawing_state.mevent.offsetY)
  //       ctx.stroke()
  //     }
  //   }
  // }

  if (drawing_state.drawing) {
    // Рисуем точку на курсоре мыши
    ctx.strokeStyle = "#000"
    ctx.fillStyle = "#000"
    ctx.beginPath()
    ctx.fillRect(
      drawing_state.mevent.clientX - wall_thickness / 2,
      drawing_state.mevent.clientY - wall_thickness / 2,
      wall_thickness,
      wall_thickness,
    )
    ctx.stroke()
  }

  const scales = [1, 2, 5]
  for (let i = -10; i !== 32; i++) {
    const decs = Math.floor(i / 3)
    const scale = scales[((i % 3) + 3) % 3]
    const meters = Math.pow(10, decs) * scale
    if (meters * zoom > 64) {
      ctx.lineWidth = 1
      ctx.strokeStyle = "#000"
      ctx.fillStyle = "#000"

      ctx.font = "bold 22px sans-serif"
      const width = Math.round(meters * zoom)
      ctx.textAlign = "center"
      ctx.fillText(`${meters}m`, 32 + width / 2, canvas.height - 50)
      ctx.fillRect(32, canvas.height - 34, width, 2)
      ctx.fillRect(32, canvas.height - 37, 2, 8)
      ctx.fillRect(32 + width, canvas.height - 37, 2, 8)
      break
    }
  }
}

// https://groups.csail.mit.edu/graphics/classes/6.838/S98/meetings/m13/bsp.html#:~:text=Let%20v%20be,return%20hit

export type Segment = {
  readonly a: Point2d,
  readonly b: Point2d,
  readonly wall: Wall,
};
export type BSP = {
  splitter: Segment
  front: BSP | undefined,
  back: BSP | undefined,
}

function resize() {
  console.log("onresize")
  canvas.width = window.innerWidth
  canvas.height = window.innerHeight
}

// const container = document.createElement("div")
// style(container, {
//   position: "absolute",
//   top: "10px",
//   left: "10px",
//   display: "flex",
//   gap: "10px"
// })
// const tools = document.createElement("div");
// tools.classList.add("menu-item")
// tools.innerText = "Tools";
// tools.addEventListener("click", (e) => {
//   if (drawing_state.drawing) {
//     drawing_state.drawing = false;
//   } else {
//     drawing_state.drawing = true;
//   }
// });
// const load = document.createElement("div")
// load.classList.add("menu-item")
// load.innerText = "Load"
// load.addEventListener("click", async (e) => {
//   const selected = await open({
//     multiple: false,
//     filters: [{
//       name: 'Config',
//       extensions: ['json']
//     }]
//   });
//   app_state.config_path = selected
//   app_state.config = await get_config(app_state.config_path)
//   await appWindow.setTitle("[" + app_state.config_path.split('\\').pop() + "] – " + "5G Planner ")
// })

// const run = document.createElement("div")
// run.classList.add("menu-item")
// run.innerText = "Run";
// run.addEventListener("click", async (e) => {
//   emit('run')
// });
// const stop = document.createElement("div")
// stop.classList.add("menu-item")
// stop.innerText = "Stop";
// stop.addEventListener("click", (e) => {
//   emit('stop')
// });
// container.appendChild(tools)
// container.appendChild(load)
// container.appendChild(run)
// container.appendChild(stop)
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
// document.body.appendChild(container);

let running = false
const btn = document.createElement("button")
btn.addEventListener("click", () => {
  if (running) {
    running = false
    btn.textContent = "Play"
  } else {
    running = true
    btn.textContent = "Stop"
  }
  should_play(running)
})
btn.textContent = "Play"
btn.style.position = "fixed"
btn.style.top = "0px"
btn.style.cursor = "pointer"
btn.style.userSelect = "none"
btn.style.fontSize = "24px"
btn.style.left = "50%"
document.body.appendChild(btn)

window.addEventListener("resize", resize)
document.addEventListener("resize", resize)
resize()

if (app_state.app_config.latest_config) {
  app_state.config_path = app_state.app_config.latest_config
  app_state.config = await get_config(app_state.config_path)
  appWindow.setTitle("[" + app_state.config_path.split("\\").pop() + "] – " + "5G Planner ")
  center_view()
  zoom = zoom_target
}

raf()

function len_vec(vec: Point2d): number {
  return Math.sqrt(vec.x * vec.x + vec.y * vec.y)
}
function normalize(vec: Point2d): Point2d {
  const len = len_vec(vec)
  return {
    x: vec.x / len,
    y: vec.y / len,
  }
}
function orthogonal(vec: Point2d): Point2d {
  return normalize({x: vec.y, y: -vec.x})
}
