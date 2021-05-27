#include "internal.h"
#include "insertion_sort.h"

void insertion_sort(void* array, size_t type_size, size_t count, Comparator comparator) {
	if (count < 2) {
		return;
	}
	uint8_t* array8 = (uint8_t*) array;
	uint8_t* end = array8 + type_size * count;
	for(uint8_t* i = array8 + type_size; i != end; i += type_size) {
		for(uint8_t* j = i; j != array8 && comparator(j-type_size, j) > 0; j -= type_size) {
			swap(j, j - type_size, type_size);
		}
	}
}
