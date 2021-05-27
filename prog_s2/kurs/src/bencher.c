#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib/insertion_sort.h"
#include "lib/heap_sort.h"

// ==== Assertion ====

void print_ints(uint8_t* array, size_t count) { 
	for(size_t i = 0; i != count; i++) {
		printf("%d ", *(array + i * sizeof(int32_t)));
	}
	printf("\n");
}

void is_sorted(void *array, size_t count, size_t size, Comparator comparator) {
	for(uint8_t *array8 = array; count > 1; array8 += size, count--) {
		if(comparator(array8, array8 + size) > 0) {
			print_ints(array8, count);
			printf("Not Sorted!\n");
			exit(EXIT_FAILURE);
		}
	}
}

// ==== Comparators, Initializers ====

int compare_ints(void* p1, void* p2) {
	return (int32_t)(*(uint32_t*)p1) - (int32_t)(*(uint32_t*)p2);
}

void int_rand_reset(void* array, size_t count) {
	for(size_t i = 0; i != count; i++) {
		((uint32_t*)array)[i] = rand()%100;
	}
}
void int_desc_reset(void* array, size_t count) {
	for(size_t i = 0; i != count; i++) {
		((uint32_t*)array)[i] = count - i;
	}
}
void int_asc_reset(void* array, size_t count) {
	for(size_t i = 0; i != count; i++) {
		((uint32_t*)array)[i] = i;
	}
}

// ==== Benchmarking code ====

enum {
	TIMEOUT = 5 * CLOCKS_PER_SEC,
};

typedef void (*SortFunc)(void*, size_t, size_t, Comparator);
typedef void (*Generator)(void*, size_t);

// ==== Testing on different sizes ====

void test_alg(SortFunc sort_func, char* alg_name, size_t limit) {
	printf("Benchmarking %s:\n", alg_name);
}

typedef struct {
	const char* name;
	SortFunc sort_func;
} Algorithm;

typedef struct {
	const char* name;
	Generator generator;
} DataType;

int main(int argc, char** argv) {
	Algorithm algorithms[2] = {
		{ .sort_func = heap_sort     , .name = "Heap Sort" },
		{ .sort_func = insertion_sort, .name = "Ins Sort"  },
	};
	DataType generators[3] = {
		{ .generator = int_asc_reset , .name = "Asc"  },
		{ .generator = int_desc_reset, .name = "Desc" },
		{ .generator = int_rand_reset, .name = "Rand" },
	};

	FILE* file = fopen("./sheet.csv", "w");
	fprintf(file, "Length ");
	for(uint8_t a = 0; a != 2; a++) {
		for(uint8_t d = 0; d != 3; d++) {
			fprintf(file, ", %s %s", algorithms[a].name, generators[d].name);
		}
	}
	fprintf(file, "\n");
	for(size_t count = 8; count <= 2<<14; count <<= 1) {
		void* array = malloc(count * sizeof(uint32_t));
		fprintf(file, "%6ld ", count);
		for(uint8_t a = 0; a != 2; a++) {
			for(uint8_t d = 0; d != 3; d++) {
				printf("Benchmarking %s %s\n", algorithms[a].name, generators[d].name);

				clock_t stop_time = clock() + TIMEOUT;
				clock_t time_total = 0;
				uint32_t ops = 0;
				while(ops != UINT32_MAX) {
					generators[d].generator(array, count);
					clock_t time0 = clock();
					algorithms[a].sort_func(array, sizeof(uint32_t), count, compare_ints);
					is_sorted(array, count, sizeof(uint32_t), compare_ints);
					clock_t time1 = clock();

					time_total += time1 - time0;
					ops++;
					
					if(time1 >= stop_time) {
						break;
					}
				}

				uint64_t ops_sec = (uint64_t)ops * CLOCKS_PER_SEC / time_total;

				fprintf(file, ", %8ld", ops_sec);
			}
		}
		fprintf(file, "\n");
	}
	fflush(file);
	fclose(file);
	return 0;
}
