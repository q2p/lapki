function getRandomInt(max) {
  return Math.floor(Math.random() * (max - 1)) + 1;
}

function mouseLeaveTitle(){
  document.getElementById("message").textContent = "Курсор вне зоны надписи"
}

function getPromiseFromEvent(item, event) {
  return new Promise((resolve) => {
    const listener = () => {
      item.removeEventListener(event, listener)
      resolve()
    }
    item.addEventListener(event, listener)
  })
}

async function exam(){
  while(1){
    document.getElementById("message").textContent = ""
    const a_num = getRandomInt(10)
    const b_num = getRandomInt(10)
    document.getElementById("title").textContent = `Введите результат умножения ${a_num} и ${b_num}`
    const elem = document.getElementById("title")
    await getPromiseFromEvent(elem, "click")
    let res = document.getElementById("res").value
    if(res == a_num * b_num){
      document.getElementById("message").textContent = "Правильно"
      break
    } 
    document.getElementById("message").textContent = "Не правильно"
    await new Promise(r => setTimeout(r, 1000))
  }
}
