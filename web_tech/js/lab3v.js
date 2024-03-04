for (let i = 0; i < 10; i++) {
  const line = document.createElement("hr")
  line.style.width = `${i * 10}%`
  document.body.appendChild(line)
}