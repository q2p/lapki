#include "internal.h"
#include <stddef.h>

void swap(uint8_t* p1, uint8_t* p2, size_t size) {
	for(size_t i = 0; i != size; i++) {
		uint8_t t = *p1;
		*p1 = *p2;
		*p2 = t;
		p1++;
		p2++;
	}
}