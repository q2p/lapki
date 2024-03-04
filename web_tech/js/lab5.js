function clickRes(){
  const cels = Number(document.getElementById("num_int").value)
  const faren = 1.8 * cels + 32
  document.getElementById("res").innerHTML = faren
}