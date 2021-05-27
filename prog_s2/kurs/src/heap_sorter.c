#include "shared.h"
#include "lib/heap_sort.h"

const char* BIN_NAME = "heap_sorter";
void sort(void* array, size_t type_size, size_t count, Comparator comparator) {
	heap_sort(array, type_size, count, comparator);
}