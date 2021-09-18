# Object
gcc -O3 -Wall -pedantic -c ./time.c
# Static
ar rs ./libtime.a ./time.o
# Shared
gcc -O3 -Wall -pedantic -shared -o ./libtime.so ./time.o
# Complile Static
# gcc -O3 -Wall -pedantic -o a.out ./main.c ./libtime.a
# Compile Shared
gcc -O3 -Wall -pedantic -o a.out ./main.c ./libtime.so
# Run
./a.out 0:33 0:12 44:32

