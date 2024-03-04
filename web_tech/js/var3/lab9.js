function countWords(){
  let text_str = document.getElementById("text_message").value
  let first_letter = `${text_str[0]}`
  let res = first_letter.toUpperCase() + text_str.substring(1)
  document.getElementById("res").textContent = res
}