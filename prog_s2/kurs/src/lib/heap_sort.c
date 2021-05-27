#include "internal.h"
#include "heap_sort.h"

void fix_heap(uint8_t* array, size_t type_size, size_t count, Comparator comparator, size_t root) {
	size_t largest = root; // Initialize largest as root
	while(1) {
		size_t l = 2 * root + 1;
		size_t r = 2 * root + 2;

		if (l < count && comparator(array + l * type_size, array + largest * type_size) > 0) {
			largest = l;
		}

		if (r < count && comparator(array + r * type_size, array + largest * type_size) > 0) {
			largest = r;
		}

		if (largest == root) {
			return;
		}

		// If largest is not root
		swap(array + root * type_size, array + largest * type_size, type_size);
		// heapify(largest);
		root = largest;
	}
}

void heap_sort(void* array, size_t type_size, size_t count, Comparator comparator) {
	if(count < 2) {
		return;
	}

	uint8_t* array8 = (uint8_t*) array;

	// Build heap (rearrange array)
	size_t i = count / 2 - 1;
	while(1) {
		fix_heap(array8, type_size, count, comparator, i);
		if(i == 0) {
			break;
		}
		i--;
	}

	// One by one extract an element from heap
	for (size_t i = count - 1; i != 0; i--) {
		swap(array8, array8 + i * type_size, type_size);
		fix_heap(array8, type_size, i, comparator, 0);
	}
}