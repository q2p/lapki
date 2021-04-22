#include "internal.h"

void sort_shaker(void* array, size_t count, size_t size, Comparator comparator) {
	uint8_t *array8 = array;
	size_t start = 0;
	size_t end = count - 1;
	uint8_t finished = 0;
	
	while(1) {
		if(finished) {
			break;    
		}

		finished = 1;
		
		for(size_t i = start; i < end; i++) {
			uint8_t* a = array8 + size * i;
			uint8_t* b = a + size;
			if (0 < comparator(a, b)) {
				swap(a, b, size);
				finished = 0;
			}
		}
		
		end--;
		
		if(finished) {
			break;    
		}

		finished = 1;
		
		for(size_t i = end; i > start; i--) {
			uint8_t* a = array8 + size * i;
			uint8_t* b = a + size;
			if (0 < comparator(a, b)) {
				swap(a, b, size);
				finished = 0;
			}
		}
		
		start++;
	}
}