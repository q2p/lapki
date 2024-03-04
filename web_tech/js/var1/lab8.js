function randomInt(max) {
  return Math.floor(Math.random() * max)
}

function makeTable(){
  const table_save = document.getElementsByTagName("table")[0]
  while (table_save.hasChildNodes()) {
    table_save.lastChild.remove()
  }
  const N = document.getElementById("N_size").value
  const M = document.getElementById("M_size").value

  let min_res = 9
  let max_res = -9
  for(let i = 0; i < N; i++){
    const row = document.createElement("tr")
    for(let j = 0; j < M; j++){
      const column = document.createElement("td")
      const res = randomInt(18) - 9
      if(res > max_res) {
        max_res = res
      } else if(res < min_res) {
        min_res = res
      }
      column.textContent = res
      row.append(column)
    }
    table_save.appendChild(row)
  }
  document.getElementById("res").innerHTML = max_res - min_res
}