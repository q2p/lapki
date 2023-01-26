#!/bin/sh
echo "Сегодня:" $(date) 
echo "Пользователь: $(whoami)"
echo "Комптютер: $(domainname -s)"
echo "Процессор:"
echo "- Модель: $(cat /proc/cpuinfo | grep -Pom1 "model name\s*:\s+\K.+")"
echo "- Архикектура: $(arch)"
echo "- Тактовая частота: $(cat /proc/cpuinfo | grep -Pom1 "cpu MHz\s+:\s+\K.+") МГц"
echo "- Количество ядер: $(cat /proc/cpuinfo | grep -Pom1 "cpu cores\s+:\s+\K.+") шт."
echo "- Потоков на ядро: $(lscpu | grep -Pom1 "Thread\(s\) per core:\s+\K.+") шт."
echo "Оперативная память:"
echo "- Всего RAM: $(($(cat /proc/meminfo | grep -Pom1 "MemTotal:\s+\K[0-9]+") / 1024)) МиБ"
echo "- Доступно RAM: $(($(cat /proc/meminfo | grep -Pom1 "MemAvailable:\s+\K[0-9]+") / 1024)) МиБ"
echo "Жёсткий диск:"
echo "- Всего ЖД: $(df -h / | grep -Pom1 "^/dev/[a-z0-9]+\s+\K.[A-Z0-9.]+")"
echo "- Свободно ЖД: $(df -h / | grep -Pom1 "^/dev/[a-z0-9]+\s+[A-Z0-9.]+\s+[A-Z0-9.]+\s+\K.[A-Z0-9.]+")"
echo "- Смонтированный раздел: /dev/$(df -h / | grep -Pom1 "^/dev/\K[a-z0-9\/]+")"
echo "- Всего SWAP: $(($(cat /proc/meminfo | grep -Pom1 "SwapTotal:\s+\K[0-9]+") / 1024)) МиБ"
echo "- Свободно SWAP: $(($(cat /proc/meminfo | grep -Pom1 "SwapFree:\s+\K[0-9]+") / 1024)) МиБ"
echo "Сетевые интерфейсы:"

echo "+-----+----------------------+-------------------+-----------------+------------+"
echo "|  №  |       Интерфейс      |        MAC        |       IP        |  Скорость  |"
echo "+-----+----------------------+-------------------+-----------------+------------+"
i=1
for f in /sys/class/net/*; do
  iface=$(basename $f)
  ipaddr=$(ip address show $iface 2> /dev/null | grep -Pom1 "inet[6]*\s\K[0-9.]+")
  if [ -n "$ipaddr" ]; then
    printf "| %-3s | %-20s | %-17s | %-15s | %-10s |\n" \
      $i \
      $iface \
      $(cat $f/address 2> /dev/null) \
      $ipaddr \
      $(ethtool $iface 2> /dev/null | grep -Pom1 "Speed:\s\K.+")
    i=$((i+1))
  fi
done
echo "+-----+----------------------+-------------------+-----------------+------------+"
