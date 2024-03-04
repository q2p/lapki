const inp = document.getElementsByTagName("input")[0]

function timesOfDay() {
  const hr = parseInt(inp.value.slice(0,2))
  let time
  switch (true) {
    case hr < 6:
      time = "Ночь"
      break
    case hr < 11:
      time = "Утро"
      break
    case hr < 16:
      time = "День"
      break
    default:
      time = "Вечер"
      break
  }
  document.getElementsByTagName("p")[0].textContent = time
}

inp.addEventListener("change", timesOfDay)

{
  const now = new Date()
  inp.value =
    `${
      now.getHours().toString().padStart(2, "0")
    }:${
      now.getMinutes().toString().padStart(2, "0")
    }`

  timesOfDay()
}