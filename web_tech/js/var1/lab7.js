function makeTable(fcolor, scolor, contents){
  const table_width = Math.sqrt(contents.length)
  const table_height = contents.length / table_width

  for(let i = 0; i < table_height; i++){
    const row = document.createElement("tr")
    let current_color = i % 2

    for(let j = 0; j < table_width; j++){
      const column = document.createElement("td")
      if(current_color){
        column.style.backgroundColor = fcolor
        current_color = 0
      } else {
        column.style.backgroundColor = scolor
        current_color = 1
      }
      column.textContent = contents[i * table_width + j]
      row.append(column)
    }
    document.getElementsByTagName("table")[0].appendChild(row)
  }
}
//[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]