#!/bin/bash
./a.exe -m RAM -b 64 -l 4
./a.exe -m RAM -b 16kb -l 4
./a.exe -m RAM -b 32kb -l 4
./a.exe -m RAM -b 512kb -l 4
./a.exe -m RAM -b 16mb -l 4
./a.exe -m RAM -b 128mb -l 4

for s in {4..64..4}
do
  ./a.exe -m SSD -b ${s}mb -l 1
done
for s in {4..64..4}
do
  ./a.exe -m HDD -b ${s}mb -l 1
done
for s in {4..64..4}
do
  ./a.exe -m flash -b ${s}mb -l 1
done