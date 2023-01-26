#!/bin/bash
echo "type; dimentions; time" >> results.csv
run() {
  echo "type: $1 dim: $2"
  time=$(./matrix.exe $1 $2)
  time=${time##Time:}
  echo "$1; $2; $time" >> results.csv
}
seq() {
  for ((d=$2; d<=$3; d*=2)); do
    for ((i=0; i!=$4; i++)); do
      run $1 $d
    done
  done
}
seq n 64 1024 2
seq n 2048 2048 1
seq o 64 2048 2
seq o 4096 8192 1
seq t 64 2048 2
seq t 4096 8192 1
