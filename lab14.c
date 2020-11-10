#include <stdio.h>
#include <stdint.h>

/// Меняет местами две переменные любого типа
void swap_any(void* a, void* b, size_t len) {
	uint8_t* c = (uint8_t*) a;
	uint8_t* d = (uint8_t*) b;
	while(len) {
		len--;
		*c ^= *d;
		*d ^= *c;
		*c ^= *d;
		c++;
		d++;
	}
}

/// Меняет местами две переменные типа `int`
void swap_int(int* a, int* b) {
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}

/// Меняет местами два указателя на любой тип
void swap_ptr(void** a, void** b) {
	// Приведение указателя к `uintptr_t` происходит без потерь,
	// так как `sizeof(uintptr_t) == sizeof(void*)`.
	// 
	// Так как размеры типов одинаковые, компилятор просто избавится от приведений,
	// и будет выполнять xor как над простыми числами без приведений.
	*((uintptr_t*) a) ^= *((uintptr_t*) b);
	*((uintptr_t*) b) ^= *((uintptr_t*) a);
	*((uintptr_t*) a) ^= *((uintptr_t*) b);
}

void debug_swap_int(unsigned int a, unsigned int b) {
	printf("(%04x, %04x) <=> ", a, b);
	swap_int(&a, &b);
	printf("(%04x, %04x)\n", a, b);
}

void debug_swap_str(char* a, char* b) {
	printf("%s %s <=> ", a, b);
	swap_ptr((void*) &a, (void*) &b);
	printf("%s %s\n", a, b);
}

int main(int argc, char** argv) {
	debug_swap_int(0x152f, 0xb69c);
	debug_swap_int(0xd4b3, 0x4948);
	debug_swap_int(0xb349, 0x4817);
	
	debug_swap_str("Twilight", "Sparkle");
	debug_swap_str("Shrek", "Donkey");

	return 0;
}
