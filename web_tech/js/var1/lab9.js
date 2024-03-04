function countLetters(){
  const message_str = document.getElementById("text_message").value
  let count_letters = 0
  for(let i = 0; i < message_str.length; i++){
    switch(message_str[i]){
      case 'У':
      case 'Е':
      case 'Ё':
      case 'Ы':
      case 'А':
      case 'О':
      case 'Э':
      case 'Я':
      case 'И':
      case 'Ю':
      case 'у':
      case 'е':
      case 'ё':
      case 'ы':
      case 'а':
      case 'о':
      case 'э':
      case 'я':
      case 'и':
      case 'ю':
        count_letters++
        break
    }
  }
  document.getElementById("res").innerHTML = count_letters
}