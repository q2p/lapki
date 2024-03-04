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

  let max_i = 0
  let max_sum_res = -9 * N
  for(let i = 0; i < N; i++){
    const row = document.createElement("tr")
    let max_sum_temp = 0
    for(let j = 0; j < M; j++){
      const column = document.createElement("td")
      const res = randomInt(18) - 9
      max_sum_temp += res
      column.textContent = res
      row.append(column)
    }
    table_save.appendChild(row)
    if(max_sum_temp > max_sum_res){
      max_sum_res = max_sum_temp
      max_i = i + 1
    }
  }
  document.getElementById("res").textContent = `Максимум из сумм элементов строк: ${max_sum_res} на ${max_i} строке`
}