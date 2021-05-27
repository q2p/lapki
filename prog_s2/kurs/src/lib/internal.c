#include "internal.h"
void swap(uint8_t* p1, uint8_t* p2, size_t type_size) {
	do {
		uint8_t t = *p1;
		*p1 = *p2;
		*p2 = t;
		p1++;
		p2++;
		type_size--;
	} while(type_size != 0);
}