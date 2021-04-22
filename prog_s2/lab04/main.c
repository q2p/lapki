#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lib/libsort.h"

// ==== Assertion ====

void is_sorted(void *array, size_t count, size_t size, Comparator comparator) {
	for(uint8_t *array8 = array; count > 1; array8 += size, count--) {
		if(comparator(array8, array8 + size) > 0) {
			printf("Not Sorted!\n");
			exit(EXIT_FAILURE);
		}
	}
}

// ==== Benchmarking code ====

enum {
	TIMEOUT = 5 * CLOCKS_PER_SEC,
};

typedef struct {
	size_t count;
	void* array;
	SortFunc sort_func;
	Comparator comparator;
} State;

uint64_t bench(void (*reset)(State*), void (*run)(State*), State* arg) {
	clock_t stop_time = clock() + TIMEOUT;
	clock_t time_total = 0;
	uint32_t ops = 0;
	while(ops != UINT32_MAX) {
		if (reset) {
			reset(arg);
		}

		clock_t time0 = clock();
		run(arg);
		clock_t time1 = clock();

		time_total += time1 - time0;
		ops++;
		
		if(time1 >= stop_time) {
			break;
		}
	}

	uint64_t ops_sec = (uint64_t)ops * CLOCKS_PER_SEC / time_total;
	return ops_sec;
}

// ==== Comparators, Initializers ====

int compare_ints(void* p1, void* p2) {
	return (int32_t)(*(uint32_t*)p1) - (int32_t)(*(uint32_t*)p2);
}
int compare_str_asc(void* p1, void* p2) {
	return strcmp(*(char**)p1, *(char**)p2);
}
int compare_str_desc(void* p1, void* p2) {
	return strcmp(*(char**)p2, *(char**)p1);
}
int compare_str_rand(void* p1, void* p2) {
	return rand() % 3 - 1;
}

void reset_str(State* state) {
	sort_binary_insertion(state->array, state->count, sizeof(char*), state->comparator);
}

void int_rand_reset(State* state) {
	for(size_t i = 0; i != state->count; i++) {
		((uint32_t*)state->array)[i] = rand()%100;
	}
}
void int_inv_reset(State* state) {
	for(size_t i = 0; i != state->count; i++) {
		((uint32_t*)state->array)[i] = state->count - i;
	}
}
void int_sorted_reset(State* state) {
	for(size_t i = 0; i != state->count; i++) {
		((uint32_t*)state->array)[i] = i;
	}
}
void str_bench(State* state) {
	state->sort_func(state->array, state->count, sizeof(char*), compare_str_asc);
	// is_sorted(state->array, state->count, sizeof(char*), compare_str_asc);
}
void int_bench(State* state) {
	state->sort_func(state->array, state->count, sizeof(uint32_t), compare_ints);
	// is_sorted(state->array, state->count, sizeof(uint32_t), compare_ints);
}

// ==== Testing on different data types and sizes ====

void test_int(SortFunc sort_func, void (*int_reset)(State*), size_t count, char* data_type) {
	printf("%7ld %s: ", count, data_type);
	State state = {
		.count = count,
		.array = malloc(sizeof(uint32_t)*count),
		.sort_func = sort_func,
		.comparator = compare_ints,
	};

	uint64_t ops = bench(int_reset, int_bench, &state);
	printf("%8ld ops / sec\n", ops);

	free(state.array);
}

void test_str(SortFunc sort_func, int (*str_comparator)(void*, void*), size_t count, char* data_type) {
	printf("%7ld %s: ", count, data_type);
	char** v = malloc(sizeof(char**)*count);
	State state = {
		.count = count,
		.array = v,
		.sort_func = sort_func,
		.comparator = str_comparator,
	};
	for(size_t i = 0; i != count; i++) {
		size_t str_len = 1 + rand() % 31;
		v[i] = (char*) malloc(str_len);
		for(size_t j = 0; j != str_len-1; j++) {
			v[i][j] = 'a' + rand() % 20;
		}
		v[i][str_len-1] = '\0';
	}

	uint64_t ops = bench(reset_str, str_bench, &state);
	printf("%8ld ops / sec\n", ops);

	for(size_t i = 0; i != count; i++) {
		free(v[i]);
	}
	free(v);
}

void test_ints(SortFunc sort_func, void (*int_reset)(State*), char* data_type, size_t limit) {
	for(size_t s = 8; s <= limit; s <<= 1) {
		test_int(sort_func, int_reset, s, data_type);
	}
}

void test_strs(SortFunc sort_func, int (*str_comparator)(void*, void*), char* data_type, size_t limit) {
	for(size_t s = 8; s <= limit >> 2; s <<= 1) {
		test_str(sort_func, str_comparator, s, data_type);
	}
}

void test_alg(SortFunc sort_func, char* alg_name, size_t limit) {
	printf("Benchmarking %s:\n", alg_name);

	test_ints(sort_func, int_sorted_reset, " ascending i32", limit);
	test_ints(sort_func, int_inv_reset,    "descending i32", limit);
	test_ints(sort_func, int_rand_reset,   "randomized i32", limit);

	test_strs(sort_func, compare_str_asc , " ascending str", limit);
	test_strs(sort_func, compare_str_desc, "descending str", limit);
	test_strs(sort_func, compare_str_rand, "randomized str", limit);

	printf("\n");
}

int main(int argc, char** argv) {
	test_alg(sort_binary_insertion, "Binary Insertion Sort", 2<<14);
	test_alg(sort_shaker,           "Cocktail Shaker Sort",  2<<12);
	return 0;
}
