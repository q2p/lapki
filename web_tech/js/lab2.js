while(true) {
  const surname = prompt("Фамилия")
  if (surname === null) {
    break;
  }
  const first_name = prompt("Имя")
  const row = document.createElement("tr")
  const column_sn = document.createElement("td")
  const column_fn = document.createElement("td")
  column_fn.textContent = first_name
  column_sn.textContent = surname
  row.append(column_sn, column_fn)
  document.getElementsByTagName("table")[0].appendChild(row)
}
