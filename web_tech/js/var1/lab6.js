let num_arr = []
let arr_str = ""
while(true) {
  const num_str = prompt("Введите числа")
  arr_str += num_str
  if (num_str === '') {
    break
  }
  arr_str += ' + '
  let num_int = parseInt(num_str)
  num_arr.push(num_int)
}
arr_str = arr_str.substring(0, arr_str.length - 3)
let num_sum = 0
for(let i = 0; i < num_arr.length; i++){
  num_sum += num_arr[i]
}
arr_str += ` = ${num_sum}`
document.getElementById("res").textContent = arr_str