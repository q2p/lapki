const students_amount = parseInt(prompt("Введите количество учеников"), 10)
for (let i = 0; i < students_amount; i++) {
  const surname = prompt("Фамилия")
  const first_name = prompt("Имя")
  const row = document.createElement("tr")
  const column_sn = document.createElement("td")
  const column_fn = document.createElement("td")
  column_fn.textContent = first_name
  column_sn.textContent = surname
  row.append(column_sn, column_fn)
  document.getElementsByTagName("table")[0].appendChild(row)
}
