for(let x = 0.0; x <= 2.0; x = x + 0.1){
  const y = 4.0*x*x - 5.5*x + 2.0
  const row = document.createElement("tr")
  const column_x = document.createElement("td")
  const column_y = document.createElement("td")
  column_x.textContent = x.toFixed(1)
  column_y.textContent = y.toFixed(2)
  row.append(column_x, column_y)
  document.getElementsByTagName("table")[0].appendChild(row)
}