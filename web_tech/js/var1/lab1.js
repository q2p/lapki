function randomInt(max) {
  const random_number = Math.floor(Math.random() * max)
  document.getElementById("rnd_num").innerHTML = random_number
  const check = random_number % 10
  if((check == 0) || (check > 4) || ((random_number > 10) && (random_number < 20))){
    document.getElementById("end_msg").textContent = "символов"
  } else if(check > 1){
    document.getElementById("end_msg").textContent = "символа"
  } else {
    document.getElementById("end_msg").textContent = "символ"
  }
}

function convertText(){
  const get_txt = document.getElementById("input_txt").value
  if(get_txt.length < parseInt(document.getElementById("rnd_num").innerHTML)){
    document.getElementById("message").textContent = "Недостаточно символов"
  } else {
    document.getElementById("message").innerHTML = "Обработанный текст"

  }
  const word_array = get_txt.split(' ')
  const temp = word_array[0]
  word_array[0] = word_array[word_array.length - 1]
  word_array[word_array.length - 1] = temp
  let res_txt = ""
  for(let i = 0; i < word_array.length; i++){
    res_txt += word_array[i] + ' '
  }
  document.getElementById("result").innerText = res_txt
}