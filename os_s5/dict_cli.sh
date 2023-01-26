#!/bin/bash

if [[ "$@" =~ ^([a-zA-Z0-9][a-zA-Z0-9-]{0,61}[a-zA-Z0-9]\.)+[a-zA-Z]{2,}$ ]]; then
  echo "Выбран сервер \"$@\""
else
  echo "Неверный параметр, программе нужен домен на вход."
  echo "Пример:"
  echo "  dict_cli.sh dict.mova.org"
  exit 1
fi

dicts_list=""
methods=""
strats=$({ echo "SHOW STRATEGIES"; sleep 1; echo "SHOW DATABASES"; sleep 1; } | timeout 5 telnet $@ 2628 2>/dev/null)
while read line; do
  if [[ "$line" == "." ]]; then
    found=""
  elif [[ $found == "1" ]]; then
    dicts_list="$dicts_list$line"$'\n'
  elif [[ $found == "2" ]]; then
    methods="$methods$line"$'\n'
  fi
  if [[ $line =~ ^110.* ]]; then
    found="1"
  elif [[ $line =~ ^111.* ]]; then
    found="2"
  fi
done <<< "$strats"

if [[ -z $methods ]]; then
  echo "Не удалось подключиться к серверу."
  exit 1
fi

function uids {
  echo "$1" | grep -oP "^\S*"
}

function uid1 {
  echo "$1" | grep -oP "\s\K[\S\s]*"
}

function find_uid {
  while read line; do
    uid=$(uids "$line")
    if [[ "$uid" == "$2" ]]; then
      uid1 "$line"
      return
    fi
  done <<< "$1"
}

function uid_by_num {
  i=1
  while read line; do
    if [[ "$i" == "$2" ]]; then
      echo "$line"
      return
    fi
    ((i++))
  done <<< "$1"
}

sel_dict=$(uid_by_num "$dicts_list" 1)
sel_dict=$(uids "$sel_dict")
sel_alg="word"
sel_word="beaver"

while true; do
  sel_dict_title=$(find_uid "$dicts_list" "$sel_dict")
  sel_alg_title=$(find_uid "$methods" "$sel_alg")
  echo "1) Словарь: $sel_dict_title"
  echo "2) Метод поиска: $sel_alg_title"
  echo "3) Слово: \"$sel_word\""
  echo "4) Искать!"
  echo "5) Выйти!"

  read opt
  case $opt in
    "1")
      echo "Словари:"
      i=1
      while read line; do
        title=$(uid1 "$line")
        if [ -n "$title" ]; then
          echo "$i) $title"
          ((i++))
        fi
      done <<< "$dicts_list"
      read opt
      found=$(uid_by_num "$dicts_list" $opt)
      if [ -n "$found" ]; then
        sel_dict=$(uids "$found")
      else
        echo "Словарь под таким номером не найден"
      fi
      ;;
    "2")
      echo "Методы:"
      i=1
      while read line; do
        title=$(uid1 "$line")
        if [ -n "$title" ]; then
          echo "$i) $title"
          ((i++))
        fi
      done <<< "$methods"
      read opt
      found=$(uid_by_num "$methods" $opt)
      if [ -n "$found" ]; then
        sel_alg=$(uids "$found")
      else
        echo "Метод под таким номером не найден"
      fi
      ;;
    "3")
      echo "Введите слово:"
      read opt
      if [ -n "$opt" ]; then
        sel_word="$opt"
      else
        echo "Слово не может быть пустым"
      fi
      ;;
    "4")
      strats=$({ echo "MATCH $sel_dict $sel_alg \"$sel_word\""; sleep 1; } | telnet $@ 2628 2>/dev/null)
      i=1
      queries=""
      found=""
      prev_dict=""
      while read line; do
        if [[ "$line" == "." ]]; then
          found=""
        elif [[ $found == "1" ]]; then
          dict=$(uids "$line")
          text=$(uid1 "$line")
          if [[ "$dict" != "$prev_dict" ]]; then
            prev_dict="$dict"
            dict_title=$(find_uid "$dicts_list" "$dict")
            echo $'\n'"$dict_title:"
          fi
          query=$(echo "$queries" | grep -oP "$line")
          if [ -n "$text" ] && [ -z "$query" ]; then
            echo "  $i) $text"
            queries="$queries$line"$'\n'
            ((i++))
          fi
        fi
        if [[ $line =~ ^152.* ]]; then
          found="1"
        fi
      done <<< "$strats"
      if [ -z "$queries" ]; then
        echo "Подходящих слов не найденно :("
      else
        query=""
        while [ -z "$query" ]; do
          read opt
          query=$(uid_by_num "$queries" $opt)
          if [ -z "$query" ]; then
            echo "Слово под таким номером не найдено"
          fi
        done
        dict=$(uids "$query")
        dict_title=$(find_uid "$dicts_list" "$dict")
        query=$(uid1 "$query")
        strats=$({ echo "DEFINE $dict $query"; sleep 1; } | telnet $@ 2628 2>/dev/null)
        while read; do
          if [[ "$REPLY" == "." ]]; then
            echo ""
            found=""
          elif [[ $found == "1" ]]; then
            echo "  $REPLY"
            ((i++))
          fi
          if [[ $REPLY =~ ^151.* ]]; then
            found="1"
            query=$(echo "$REPLY" | grep -oP "\"\K[^\"]*" | head -n 1)
            echo "Определение \"$query\" Из Словаря $dict_title:"$'\n'
          fi
        done <<< "$strats"
      fi
      ;;
    "5")
      break
      ;;
    *)
      echo "Пожалуйста выберите комманду из доступных"
      ;;
  esac
done

echo "Досвидания!"
