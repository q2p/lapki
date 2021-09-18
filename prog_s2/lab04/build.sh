mkdir -p ./tmp

# Build library
gcc -O3 -Wall -pedantic -c ./lib/shared.c                    -o ./tmp/shared.o
gcc -O3 -Wall -pedantic -c ./lib/sort_alg_shaker.c           -o ./tmp/sort_alg_shaker.o
gcc -O3 -Wall -pedantic -c ./lib/sort_alg_binary_insertion.c -o ./tmp/sort_alg_binary_insertion.o
gcc -O3 -Wall -pedantic -c ./lib/sort_alg_quick_sort.c       -o ./tmp/sort_alg_quick_sort.o
ar rs ./tmp/libsort.a      ./tmp/shared.o\
                           ./tmp/sort_alg_shaker.o\
                           ./tmp/sort_alg_binary_insertion.o\
                           ./tmp/sort_alg_quick_sort.o

# Build binary
gcc -O3 -Wall -pedantic -o bencher -march=native -mtune=native ./main.c ./tmp/libsort.a

# Clean up
rm -rf ./tmp/

# Run
./bencher