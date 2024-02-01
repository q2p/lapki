const back = document.createElement("div")
back.classList.add("background")
document.body.insertBefore(back, document.body.firstChild)

const EMOJIS = ["🔥","🤡","👌","🍍","🙏","😇","💯"]
let RAD = 1
let prev = 1
let last = 1
const particles = []
let W = 1
let H = 1

function rand(from, to) {
  return from + Math.random() * (to - from)
}

class Particle {
  constructor() {
    this.y = -RAD;
    this.x = rand(-RAD, W + RAD)
    this.dy = rand(0.9, 1.2) * (H + RAD)
    const shift = Math.max(-1, Math.min(1, 2 * (this.x / W) - 1))
    this.dx = W * (rand(-1, 1) - Math.pow(shift, 10)) * 0.5
    this.el = document.createElement("div")
    this.el.classList.add("particle")
    this.el.textContent = EMOJIS[Math.floor(Math.random() * EMOJIS.length)]
    back.appendChild(this.el)
    RAD = 4 * this.el.clientHeight
    this.el.style.animation = `spin ${rand(1, 3).toFixed(2)}s infinite linear`
  }
  update(delta) {
    this.dy -= delta * H * 0.8
    this.x += this.dx * delta
    this.y += this.dy * delta
    this.el.style.bottom = `${this.y}px`
    this.el.style.left = `${this.x}px`
    return this.y < -2 * RAD
  }
}

function raf() {
  W = document.body.parentElement.clientWidth
  H = document.body.parentElement.clientHeight
  requestAnimationFrame(raf)
  const now = new Date().getTime() * 0.001
  const delta = now - prev
  prev = now
  if (particles.length < 40 && now - last > 0.2) {
    particles.push(new Particle())
    last = now
  }
  for (let i = 0; i < particles.length;) {
    if (particles[i].update(delta)) {
      particles.splice(i, 1)[0].el.remove()
    } else {
      i++
    }
  }
}
requestAnimationFrame(raf)
