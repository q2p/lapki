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

window.addEventListener("keydown", async function(e) {
  if (e.code === "KeyS") {
    image.src = "../rimg3.png"
    center_view()
    BB = await get_bb()
  }
})

function mk_tooltip(x: number, y: number, color: string, text: string) {
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
  box.textContent = text
  box.style.position = "absolute"
  box.style.background = "rgba(33,33,33,0.7)"
  box.style.color = "white"
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
      `Pow: ${el.point_pow_mw.toPrecision(4)}mw`,
    )
    mk_tooltip(
      el.min_sinr_x,
      el.min_sinr_y,
      color,
      `Min SINR: ${el.min_sinr_dbm.toPrecision(4)}dbm`,
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

  // TODO: CHECK IF UNDEFINED OR NUL
  ctx.lineWidth = wall_thickness
  ctx.strokeStyle = "#000"
  ctx.fillStyle = "#000"
  ctx.beginPath()
  for (const w of app_state.config.walls) {
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

    // Раскомментить для угловатых стен.
    // w.a.x = Math.round(w.a.x)
    // w.a.y = Math.round(w.a.y)
    // w.b.x = Math.round(w.b.x)
    // w.b.y = Math.round(w.b.y)
    // ctx.moveTo((w.a.x-offsetX)*zoom, (w.a.y-offsetY)*zoom);
    // ctx.lineTo((w.b.x-offsetX)*zoom, (w.b.y-offsetY)*zoom);
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

  render_bsp()
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
const ray1: Point2d = { x: 0, y: 0 }
const ray2: Point2d = { x: 0, y: 0 }
function render_bsp() {
  if (bsp === undefined) {
    build_bsp()
  }
  const offsetX = camX - canvas.width / (2 * zoom)
  const offsetY = camY - canvas.height / (2 * zoom)
  ray2.x = cursor_x_m
  ray2.y = cursor_y_m

  ctx.lineWidth = 4
  ctx.fillStyle = "#050"
  function render_node(bsp: BSP | undefined, depth: number, dir,): void {
    if (bsp === undefined) {
      return
    }
    const ax = zoom * (bsp.splitter.a.x - offsetX)
    const ay = zoom * (bsp.splitter.a.y - offsetY)
    const bx = zoom * (bsp.splitter.b.x - offsetX)
    const by = zoom * (bsp.splitter.b.y - offsetY)
    ctx.fillStyle = ctx.strokeStyle = `hsl(${depth * 120} 80 ${20 + depth * 10})`
    if (len_vec(sub_vec(ray2, bsp.splitter.a)) < 0.1) {
      ctx.fillStyle = ctx.strokeStyle = `hsl(0 80 100)`
    }
    ctx.beginPath()
    ctx.moveTo(ax, ay)
    ctx.lineTo(bx, by)
    const splitter_mid = mul_vec(add_vec(bsp.splitter.a, bsp.splitter.b), 0.5)
    const splitter_dir = sub_vec(bsp.splitter.a, bsp.splitter.b)
    const splitter_norm = add_vec(splitter_mid, mul_vec(orthogonal(splitter_dir), 0.1))
    ctx.moveTo(zoom * (splitter_mid.x - offsetX), zoom * (splitter_mid.y - offsetY))
    ctx.lineTo(zoom * (splitter_norm.x - offsetX), zoom * (splitter_norm.y - offsetY))
    ctx.closePath()
    ctx.stroke()
    ctx.arc(ax, ay, 5, 0, 2 * Math.PI);
    ctx.closePath()
    ctx.arc(bx, by, 5, 0, 2 * Math.PI);
    ctx.fill()
    ctx.textAlign = "center"
    ctx.font = "bold 18px sans-serif"
    ctx.fillStyle = "#222"
    ctx.fillText(dir, zoom * (splitter_norm.x - offsetX), zoom * (splitter_norm.y - offsetY))
    render_node(bsp.front, depth + 1, dir + "f")
    render_node(bsp.back, depth + 1, dir + "b")
  }

  render_node(bsp, 0, "_")

  ctx.beginPath();
  // ctx.ellipse(ray1.x - camX, ray1.y - camY, 16, 16, 0, 0, 180)
  ctx.arc((ray1.x - offsetX) * zoom, (ray1.y - offsetY) * zoom, 8, 0, 2 * Math.PI);
  ctx.arc((ray2.x - offsetX) * zoom, (ray2.y - offsetY) * zoom, 8, 0, 2 * Math.PI);
  const int = intersect(bsp, ray1, ray2)
  if (int !== undefined) {
    ctx.arc((int.x - offsetX) * zoom, (int.y - offsetY) * zoom, 8, 0, 2 * Math.PI);
  }
  ctx.fill()
}

let bsp: BSP | undefined = undefined
function intersect(bsp: BSP, r0: Point2d, r1: Point2d): Point2d | undefined {

  const q = r0
  const p = bsp.splitter.a
  const s = sub_vec(r1, r0)
  const r = sub_vec(bsp.splitter.b, bsp.splitter.a)
  // t = (q − p) × s / (r × s)
  // u = (q − p) × r / (r × s)
  const qp = sub_vec(q, p)
  const rs = cross_prod(r, s)
  const rs_zero = Math.abs(rs) < 0.00001
  const qps = cross_prod(qp, s)
  const qpr = cross_prod(qp, r)

  // const ray_dir = sub_vec(r1, r0)
  // const segment_dir = sub_vec(bsp.splitter.b, bsp.splitter.a)
  // const numerator = cross_prod(sub_vec(bsp.splitter.a, r0), ray_dir)
  // const denominator = cross_prod(ray_dir, segment_dir)

  // const numerator_is_zero = Math.abs(numerator) < 0.00001

  let near: BSP | undefined
  let far: BSP | undefined
  // if (numerator < 0 || (numerator_is_zero && denominator > 0)) {
  if (qpr < 0 || (Math.abs(qpr) < 0.00001 && rs > 0)) {
    near = bsp.front
    far = bsp.back
  } else {
    near = bsp.back
    far = bsp.front
  }
  if (near !== undefined) {
    const hit = intersect(near, r0, r1)
    if (hit !== undefined) {
      return hit
    }
  }

  // if the denominator is zero the lines are parallel
  // if (Math.abs(denominator) < 0.00001) {
  if (rs_zero) {
    return undefined
  }

  const t = qps / rs
  const u = qpr / rs

  // intersection is the point on a line segment where the line divides it
  // const intersection = numerator / denominator

  // segments that are not parallel and t is in (0, 1) should be divided
  // if (0.0 < intersection && intersection < 1.0) {
  //   return add_vec(bsp.splitter.a, mul_vec(segment_dir, intersection))
  // }
  if (u > 0 && 0 < t && t < 1) {
    return add_vec(q, mul_vec(s, u))
  }

  if (far !== undefined) {
    return intersect(far, r0, r1)
  }
  return undefined
}
function build_bsp() {
  if (app_state.config !== undefined) {
    const segments = app_state.config.walls.map((w) => { return {a: w.a, b: w.b, wall: w } })
    bsp = build_sub_tree(segments)
    console.dir(bsp)
  }
}
function build_sub_tree(segments: Segment[]): BSP | undefined {
  if (segments.length === 0) {
    return undefined
  }

  let best_front: Segment[]  = []
  let best_back: Segment[] = []
  let best_splitter: Segment = segments[0]

  for (let i = 256; i !== 0; i--) {
    const [splitter, front, back] = bsp_split(segments)

    const imbalance = Math.abs(front.length - back.length)
    const best_imbalance = Math.abs(best_front.length - best_back.length)
    if (best_front.length + best_back.length === 0 || imbalance < best_imbalance || best_front.length + best_back.length > front.length + back.length) {
      best_front = front
      best_back = back
      best_splitter = splitter
    }
  }

  return {
    splitter: best_splitter,
    front: build_sub_tree(best_front),
    back: build_sub_tree(best_back),
  }
}
function bsp_split(segments: Segment[]): [Segment, Segment[], Segment[]] {
  const splitter = segments[Math.floor(Math.random() * segments.length)]
  const splitter_dir = sub_vec(splitter.b, splitter.a)

  const front_segments: Segment[] = []
  const back_segments: Segment[] = []

  for (const segment of segments) {
    if (Object.is(segment, splitter)) {
      continue
    }
    const segment_dir = sub_vec(segment.b, segment.a)
    const numerator = cross_prod(sub_vec(segment.a, splitter.a), splitter_dir)
    const denominator = cross_prod(splitter_dir, segment_dir)

    // if the denominator is zero the lines are parallel
    const denominator_is_zero = Math.abs(denominator) < 0.00001

    // segments are collinear if they are parallel and the numerator is zero
    const numerator_is_zero = Math.abs(numerator) < 0.00001

    if (!denominator_is_zero) {
      // intersection is the point on a line segment where the line divides it
      const intersection = numerator / denominator

      // segments that are not parallel and t is in (0, 1) should be divided
      if (0.0 < intersection && intersection < 1.0) {
        const intersection_point = add_vec(segment.a, mul_vec(segment_dir, intersection))

        let r_segment = {
          a: segment.a,
          b: intersection_point,
          wall: segment.wall,
        }

        let l_segment = {
          a: intersection_point,
          b: segment.b,
          wall: segment.wall,
        }

        if (numerator > 0) {
          const t = l_segment
          l_segment = r_segment
          r_segment = t
        }

        front_segments.push(r_segment)
        back_segments.push(l_segment)
        continue
      }
    }

    if (numerator < 0 || (numerator_is_zero && denominator > 0)) {
      front_segments.push(segment)
    } else {
      back_segments.push(segment)
    }
  }
  return [splitter, front_segments, back_segments]
}
requestAnimationFrame(build_bsp)

window.addEventListener("keydown", (e) => {
  if (e.code === "Space") {
    ray1.x = ray2.x
    ray1.y = ray2.y
  }
})

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
