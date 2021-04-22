#include "internal.h"
#include <string.h>
#include <alloca.h>

void sort_binary_insertion(void* array, size_t count, size_t size, Comparator comparator) {
	void* temp = alloca(size);
	uint8_t* array8 = (uint8_t*) array;
	uint8_t* next = array8 + size;
	for (size_t i = 1; i != count; i++, next += size) {
		ptrdiff_t s = 0;
		ptrdiff_t e = i-1;
		while (s <= e) {
			size_t mid = ((size_t)s + (size_t)e) / 2;

			if (comparator(next, array8 + size * mid) > 0) {
				s = (ptrdiff_t)mid + 1;
			} else {
				e = (ptrdiff_t)mid - 1;
			}
		}

		size_t ip = s;

		uint8_t* ip_ptr = array8 + size * ip;

		memcpy(temp, next, size);
		memmove(ip_ptr + size, ip_ptr, size*(i-ip));
		memcpy(ip_ptr, temp, size);
	}
}
