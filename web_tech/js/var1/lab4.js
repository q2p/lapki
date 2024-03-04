function check_moosh(){
  const moosh_color = document.getElementById("mooshrooms").value
  var checkbox = document.getElementById("skirt");
  var isChecked = checkbox.checked;
  switch(moosh_color){
    case 'white':
      if(isChecked){
        document.getElementById("res").textContent = "С грибами надо быть внимательнее"
        break
      }
      document.getElementById("res").textContent = "Вы нашли белый гриб"
      break
    case 'red':
      if(isChecked){
        document.getElementById("res").textContent = "А не мухомор ли это?"
        break
      }
      document.getElementById("res").textContent = "Может, это подосиновик"
      break
    default:
      document.getElementById("res").textContent = "С грибами надо быть внимательнее"
  }
}
check_moosh()