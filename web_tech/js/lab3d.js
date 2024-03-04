const lines = []
for (let i = 0; i < 100; i++) {
  const line = document.createElement("hr")
  lines.push(line)
  document.body.appendChild(line)
}

function redraw() {
  requestAnimationFrame(redraw)
  const now = new Date().getTime()
  for (let i = 0; i < lines.length; i++) {
    const sin = (Math.sin(i*0.5 + now*0.001) + Math.sin(i * 0.8 + now * 0.0013))
    lines[i].style.width = `${(0.1 + 0.2 * (sin + 1)) * 100}%`
  }
}
redraw()