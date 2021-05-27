mkdir -p ./bin
mkdir -p ./tmp
# Build library
gcc -O3 -Wall -pedantic -o ./tmp/internal.o       -c ./src/lib/internal.c
gcc -O3 -Wall -pedantic -o ./tmp/heap_sort.o      -c ./src/lib/heap_sort.c
gcc -O3 -Wall -pedantic -o ./tmp/insertion_sort.o -c ./src/lib/insertion_sort.c 
gcc -O3 -Wall -pedantic -o ./bin/libsort.so       -shared\
                           ./tmp/internal.o\
                           ./tmp/heap_sort.o\
                           ./tmp/insertion_sort.o
# Build binaries
gcc -O3 -Wall -pedantic -o ./bin/bencher          -march=native -mtune=native ./src/bencher.c                                ./bin/libsort.so
gcc -O3 -Wall -pedantic -o ./bin/heap_sorter      -march=native -mtune=native ./src/simple_sorter.c      ./src/heap_sorter.c ./bin/libsort.so
gcc -O3 -Wall -pedantic -o ./bin/insertion_sorter -march=native -mtune=native ./src/simple_sorter.c ./src/insertion_sorter.c ./bin/libsort.so
# Clean up
rm -rf ./tmp/