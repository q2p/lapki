const back = document.createElement("div")
const circle = document.createElement("div")
const scanlines = document.createElement("div")
const logo = document.createElement("div")
circle.classList.add("circle")
back.classList.add("background")
scanlines.classList.add("scanlines")
logo.classList.add("logo")
back.append(circle, logo, scanlines);
document.body.insertBefore(back, document.body.firstChild)

const CATCH = 2
const DIMMING = 0.1
const MAX_OPACITY = 0.2

let prev = 1

class Coord {
  constructor() {
    this.old = 0
    this.cur = 0
  }
  pixels() {
    return `${this.old.toFixed(2)}px`
  }
  calc(delta) {
    delta = Math.min(1, delta * CATCH)
    this.old = this.cur * delta + this.old * (1-delta)
  }
}
function opacity() {
  const fade_dist = Math.min(window.screen.width, window.screen.height) * DIMMING
  const x2 = x.cur - x.old
  const y2 = y.cur - y.old
  const dist = x2*x2+y2*y2
  let ret = Math.pow(Math.min(1, Math.sqrt(dist, 0.3) / fade_dist), 0.8) * MAX_OPACITY
  if (ret < 0.02) {
    return 0
  }
  return ret
}

function raf() {
  requestAnimationFrame(raf)
  const now = new Date().getTime() * 0.001
  const delta = now - prev
  prev = now
  x.calc(delta)
  y.calc(delta)
  circle.style.left = x.pixels()
  circle.style.top  = y.pixels()
}
requestAnimationFrame(raf)

let x = new Coord()
let y = new Coord()
document.addEventListener("mousemove", function(e) {
  x.cur = e.clientX
  y.cur = e.clientY
})
