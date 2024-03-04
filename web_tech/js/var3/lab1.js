const max = 45
const random_number = Math.floor(Math.random() * (max - 1)) + 1
document.getElementById("rnd_num").innerHTML = random_number
const check = random_number % 10
if((check > 4) || ((random_number > 10) && (random_number < 20))){
  document.getElementById("end_msg").textContent = "символов"
} else if(check > 1){
  document.getElementById("end_msg").textContent = "символа"
} else {
  document.getElementById("end_msg").textContent = "символ"
}


function convertText(){
  const get_txt = document.getElementById("input_txt").value
  if(get_txt.length < parseInt(document.getElementById("rnd_num").innerHTML)){
    document.getElementById("message").textContent = "Недостаточно символов"
  } else {
    document.getElementById("message").textContent = "Обработанный текст"
  }
  let res_txt = ""
  for(let i = 0; i < get_txt.length; i++){
    if(i % 2 === 1){
      res_txt += get_txt[i].toUpperCase()
    } else {
      res_txt += get_txt[i].toLowerCase()
    }
  }
  
  document.getElementById("result").innerText = res_txt
}