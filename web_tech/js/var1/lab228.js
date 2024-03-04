const words = [
  "privet",
  "poka",
  "dada",
]
class Particle {
  constructor() {
    this.word = words[Math.random() * words.length]
    this.el = document.createElement("div")
    document.body.appendChild(this.el)
  }
}
let prev = 0

const SPAWN_RATE = 0.1
let next_spawn = 0

function raf() {
  requestAnimationFrame(raf)
  const now = performance.now() / 1000
  delta = now - prev
  prev = now
  if (next_spawn > now) {
    new Particle()
    next_spawn = Math.max(next_spawn + SPAWN_RATE)
  }
}
requestAnimationFrame(raf)
