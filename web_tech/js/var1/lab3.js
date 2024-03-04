const max = parseInt(prompt("Введите до какого числа найти простые числа"))
let prime_arr = [2]
let prime_count = 1
let prime_str = "2, "
for(let i = 2; i < max; i++){
  let not_prime = 0
  for(let j = 0; j < prime_arr.length; j++){
    if(i % prime_arr[j] == 0){
      not_prime = 1
      break
    }
  }
  if(not_prime == 0){ 
    prime_arr[prime_count] = i
    prime_count++
    prime_str += `${i}, `
  }
}
document.getElementById("prm_str").textContent = prime_str