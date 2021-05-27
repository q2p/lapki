#include "shared.h"
#include "lib/insertion_sort.h"

const char* BIN_NAME = "insertion_sorter";
void sort(void* array, size_t type_size, size_t count, Comparator comparator) {
	insertion_sort(array, type_size, count, comparator);
}