#!/bin/bash

case $@ in
  dict.org)
    mova=false
    ;;
  dict.mova.org)
    mova=true
    ;;
  *)    
    echo "Неверный параметр. Используйте \"dict.org\" или \"dict.mova.org\""
    exit 1
    ;;
esac

if [[ $mova = true ]]; then
  html=$(curl -s http://dict.mova.org/)
else
  html=$(curl -s http://dict.org/bin/Dict/)
  dicts=$(echo "$html" | grep -zoP "name=\"Database\"[\S\s]*</select>" | tr -d '\0')

  dicts_list=""
  while read line; do
    name=$(echo "$line" | grep -oP "<option value=\"\K[^\"]*")
    title=$(echo "$line" | grep -oP ">\K.*")
    if [ -n "$name" ] && [[ "$name" != "*" ]] && [[ "$name" != "!" ]]; then
      dicts_list="$dicts_list$name - $title"$'\n'
    fi
  done <<< "$dicts"
  dicts_list=$(tr -d '\0' <<< "$dicts_list")
fi

methods="exact - Точное совпадение (Суслик -> Суслик)
word - Совпадение по слову (Суслик -> Суслик, Лесной Суслик)
substring - Частичное совпадение (Ус -> Усы, Суслик)
re - Регулярные выражение (C.*ик -> Суслик, Супик)
lev - Расстояние по Левинштайну (Сслк -> Суслик)"

function uids {
  echo "$1" | grep -oP "^\S*"
}

function uid1 {
  echo "$1" | grep -oP " - \K[\S\s]*"
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

# Form=Dict1&Query=WORD&Strategy=lev&Database=*&submit=Submit+query

echo $mova

sel_dict="all"
sel_alg="word"
sel_word="beaver"

while true; do
  sel_dict_title=$(find_uid "$dicts_list" "$sel_dict")
  sel_alg_title=$(find_uid "$methods" "$sel_alg")
  echo "1) Словарь: $sel_dict_title"
  echo "2) Метод поиска: $sel_alg_title"
  echo "3) Слово: $sel_word"
  echo "4) Искать!"
  echo "5) Выйти!"

  read opt
  case $opt in
    "1")
      echo "Словари:"
      i=1
      while read line; do
        title=$(uid1 "$line")
        echo -e "$i) $title"
        ((i++))
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
        echo -e "$i) $title"
        ((i++))
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
      html=$(curl -s -X POST \
        -d 'Form=Dict1' \
        -d "Query=$sel_word" \
        -d "Strategy=$sel_alg" \
        -d "Database=$sel_dict" \
        -d "submit=Submit+query" \
        http://dict.org/bin/Dict/)
      html=$(echo "$html" | grep -zoP "<pre>\K[\S\s]*</pre>" | tr -d '\0')
      i=1
      queries=""
      while read line; do
        dict=$(echo "$line" | grep -oP "<b>\K[^:]*")
        if [ -n "$dict" ]; then
          dict_title=$(find_uid "$dicts_list" "$dict")
          echo $'\n'"$dict_title:"
        fi
        
        line=$(echo "$line" | grep -oP "<a href=\"\K[^\\<]*")
        while read line; do
          query=$(echo "$line" | grep -oP "^[^\"]*")
          text=$(echo "$line" | grep -oP ">\K[\S\s]*")
          found=$(find_uid "$queries" "$query")
          if [ -n "$text" ] && [ -z "$found" ]; then
            echo "  $i) $text"
            queries="$queries$query - $text"$'\n'
            ((i++))
          fi
        done <<< "$line"
      done <<< "$html"
      if [ -z queries ]; then
        echo "Подходящих слов не найденно :("
      else
        query=""
        while [ -z "$query" ]; do
          read opt
          found=$(uid_by_num "$queries" $opt)
          if [ -n "$found" ]; then
            query=$(uids "$found")
          else
            echo "Слово под таким номером не найдено"
          fi
        done
        dict=$(echo $query | grep -oP "Database=\K[^&]*")
        dict_title=$(find_uid "$dicts_list" "$dict")
        query=$(echo $query | grep -oP "Query=\K[^&]*")
        html=$(curl -s -X POST \
          -d 'Form=Dict2' \
          -d "Query=$query" \
          -d "Database=$dict" \
          -d "submit=Submit+query" \
          http://dict.org/bin/Dict/)
        html=$(echo "$html" | grep -zoP "<pre>[\S\s]*?</pre>" | tr -d '\0')
        while read; do
          pre=$(echo "$REPLY" | grep -o "<pre>")
          prepre=$(echo "$REPLY" | grep -o "<pre></pre>")
          if [ -n "$pre" ] && [ -z "$prepre" ]; then
            echo "Из Словаря $dict_title:"
          fi
          echo "$REPLY" | sed -e 's/<[^>]*>//g'
        done <<< "$html"
        
      fi
      ;;
    "5")
      break
      ;;
    *)
      echo "Выберите комманду из доступных"
      ;;
  esac
done

echo "Досвидания!"
