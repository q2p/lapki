#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "shared.h"

int compare_ints(void* p1, void* p2) {
	return (int32_t)(*(uint32_t*)p1) - (int32_t)(*(uint32_t*)p2);
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [32-bit integers]\n", BIN_NAME);
		return 1;
	}
	size_t count = argc-1;
	int32_t* array = (int32_t*)malloc(count * sizeof(int32_t));
	for(size_t i = 0; i != count; i++) {
		char * end_ptr;
		array[i] = strtoll(argv[1+i], &end_ptr, 10);
		if(*end_ptr != '\0') {
			fprintf(stderr, "Invalid argument \"%s\". Usage: %s [32-bit integers]\n", argv[1+i], BIN_NAME);
			return 1;
		}
	}
	sort(array, sizeof(int32_t), count, compare_ints);
	
	printf("Sorted numbers:");
	for(size_t i = 0; i != count; i++) {
		printf(" %d", array[i]);
	}
	printf("\n");
	return 0;
}
