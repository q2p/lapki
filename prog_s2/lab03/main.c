// Object
// gcc -O3 -Wall -pedantic -c ./time.c
//
// Static
// ar rs ./libtime.a ./time.o
//
// Shared
// gcc -O3 -Wall -pedantic -shared -o ./libtime.so ./time.o
//
// Complile
// gcc -O3 -Wall -pedantic -o a.out ./main.c ./libtime.a
// gcc -O3 -Wall -pedantic -o a.out ./main.c ./libtime.so
//
// Run
// ./a.out 0:33 0:12 44:32

#include "stdio.h"
#include "stdlib.h"
#include "time.h"

int main(int argc, char** argv) {
	if (argc == 1) {
		printf("Please specify time durations in HH:MM format.\n");
		return 1;
	}
	size_t amount_of_durations = argc - 1;
	Time** durations = malloc(amount_of_durations * sizeof(Time*));
	for(size_t i = 0; i != amount_of_durations; i++) {
		Time* duration = time_from_hm_string(argv[1+i]);
		if (!duration) {
			printf("Error: duration \"%s\" is malformed.", argv[1+i]);
			for(size_t j = 0; j != i; j++) {
				time_destroy(durations[j]);
			}
			free(durations);
			return 1;
		}
		durations[i] = duration;
	}

	Time* min_time = time_smallest(durations, amount_of_durations);
	Time* total_time = time_total(durations, amount_of_durations);
	Time* diff = time_elapsed(min_time, total_time);

	printf("Total duration is ");
	time_print(total_time);
	printf(". Smallest duration is ");
	time_print(min_time);
	printf(". Difference between total duration and smallest one is ");
	time_print(diff);
	printf(".\n");

	time_destroy(total_time);
	time_destroy(diff);
	for(size_t i = 0; i != amount_of_durations; i++) {
		time_destroy(durations[i]);
	}
	free(durations);
	return 0;
}