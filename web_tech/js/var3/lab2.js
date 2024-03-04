while(1){
  const num_inp_str = prompt("Введите число от 15 до 90")
  if(num_inp_str === null){
    break
  } 
  num_inp = parseInt(num_inp_str)
  if(isNaN(num_inp_str)){
    alert("Введено не число")
    continue
  } else if(num_inp < 15){
    alert("Введено число меньше 15")
    continue
  } else if(num_inp > 90){
    alert("Введено число больше 90")
    continue
  }
  
  const txt_inp = prompt("Введите текст длинной не менее 10 символов")
  if(txt_inp.length < 10){
    alert("Введён текст длинной менее 10")
    continue
  } 
  break
}