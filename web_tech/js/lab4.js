const min_value = 1
const max_value = 10

const row = document.createElement("tr")
for (let i = min_value; i < max_value; i++) {
  const value = document.createElement("th")
  value.textContent = i
  row.append(value)
}
document.getElementsByTagName("table")[0].appendChild(row)

for (let i = min_value + 1; i < max_value; i++) {
  const row = document.createElement("tr")
  const value = document.createElement("th")
  value.textContent = i
  row.append(value)
  
  for (let j = min_value + 1; j < max_value; j++) {
    const expression = document.createElement("td")
    expression.textContent = i * j
    row.append(expression)
  }
  document.getElementsByTagName("table")[0].appendChild(row)
}