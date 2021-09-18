#include "internal.h"

static void quick_sort(uint8_t *array, size_t size, Comparator comparator, size_t first, size_t last) {
	if (first >= last) {
		return;
	}
	uint8_t* middle = array + ((first + last) / 2) * size;
	size_t left = first;
	size_t right = last;
	do {
		while(comparator(array + (left * size), middle) < 0) {
			left++;
		}
		while(comparator(array + (right * size), middle) > 0) {
			right--;
		}
		if (left <= right) {
			swap(array + (left * size), array + (right * size), size);
			left++;
			right--;
		}
	} while (left <= right);
	quick_sort(array, size, comparator, first, right);
	quick_sort(array, size, comparator,  left,  last);
}

void sort_quick_sort(void* array, size_t count, size_t size, Comparator comparator) {
	quick_sort((uint8_t*) array, size, comparator, 0, count-1);
}