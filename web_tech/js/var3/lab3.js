const a_mod = 5
const b_mod = 11
const last_iter = 100 / a_mod
let res_str = ""
for(let i = 0; i < last_iter; i++){
  const temp = a_mod * i + 2
  if(temp % b_mod === 2){
    res_str += `${temp}, `
  }
}
document.getElementById("res").textContent = res_str