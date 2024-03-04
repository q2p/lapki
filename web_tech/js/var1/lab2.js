function randomInt(max) {
  return Math.floor(Math.random() * max)
}

let num_usr = parseInt(prompt("Введите число"))
while(true){
  num_rnd = randomInt(100)
  document.getElementById("usr").innerHTML = num_usr
  document.getElementById("mlt").textContent = " * "
  document.getElementById("rnd").innerHTML = num_rnd
  document.getElementById("eql").textContent = " = "
  document.getElementById("res").innerHTML = num_usr * num_rnd
  if(num_usr * num_rnd > 100) {
    break
  }
  num_usr = parseInt(prompt("Введите число ещё раз"))
}