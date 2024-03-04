function func(arr, a, b){
  let cutttedarr = []
  const lastiter = b - a
  for(let i = 0; i < lastiter; i++){
    cutttedarr[i] = arr[a + i]
  }
  return cutttedarr
}

function cutArr(){
  let string = document.getElementById("arr_str").value
  let array = string.split(', ')
  let a_point = document.getElementById("a_point").value
  let b_point = document.getElementById("b_point").value
  let res = func(array, parseInt(a_point), parseInt(b_point))
  document.getElementById("res").textContent = res.join(", ")
}
//1, 4, 48, 37, 18, 29, 90, 177, 4, 7, 9, 2, 0