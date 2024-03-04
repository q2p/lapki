let nfact = 1
for(let n = 1; n <= 30; n++){
  nfact = n * nfact
  const row = document.createElement("tr")
  const column_n = document.createElement("td")
  const column_nfact = document.createElement("td")
  column_n.textContent = n
  column_nfact.textContent = nfact
  row.append(column_n, column_nfact)
  document.getElementsByTagName("table")[0].appendChild(row)
}