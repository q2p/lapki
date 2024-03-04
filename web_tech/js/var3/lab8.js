function countWords(){
  let words_str = document.getElementById("text_message").value
  let words_arr = words_str.split(' ')
  let count_words = words_arr.length
  document.getElementById("res").textContent = `Количество слов в тексте: ${count_words}`
}