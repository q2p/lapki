// gcc main.c -O3 -march=native -ffast-math -s && ./a.out ./a.out

#pragma GCC optimize ("O3")

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define BUFFER_SIZE_U8  (100)
#define BUFFER_SIZE_ULL (BUFFER_SIZE_U8 / sizeof(ULL) + 1)

// #define LUT
#define CHECKS

typedef unsigned long long ULL;

ULL buffer[BUFFER_SIZE_ULL];

size_t count = 0;
size_t filled = 0;
size_t read = 0;

#ifndef LUT
inline void count_ones() {
	ULL*     c1 =            buffer                          ;
	ULL*     e1 =            buffer  + (filled / sizeof(ULL));
	uint8_t* e2 = ((uint8_t*)buffer) +  filled               ;
	while(c1 != e1) {
		count += __builtin_popcountll(*c1);
		c1++;
	}
	uint8_t* c2 = (uint8_t*)c1;
	while(c2 != e2) {
		count += __builtin_popcount((unsigned int)(*c2));
		c2++;
	}
}
#endif

#ifdef LUT
uint8_t ones_lut[0x10000];

inline void build_lut() {
	ones_lut[0] = 0;
	for(uint16_t i = 1; i != 0; i++) {
		uint8_t ones = 0;
		uint16_t j = i;
		while(j != 0) {
			ones += j & 1;
			j >>= 1;
		}
		ones_lut[i] = ones;
	}
}

inline void count_ones() {
	uint16_t* c1 = ((uint16_t*)buffer)                              ;
	uint16_t* e1 = ((uint16_t*)buffer) + (filled / sizeof(uint16_t));
	uint8_t*  e2 = ((uint8_t* )buffer) +  filled                    ;
	while(c1 != e1) {
		count += ones_lut[*c1];
		c1++;
	}
	uint8_t* c2 = (uint8_t*)c1;
	while(c2 != e2) {
		count += ones_lut[*c2];
		c2++;
	}
}
#endif

int main(int argc, char** argv) {
	#ifdef LUT
	build_lut();
	#endif

	#ifdef CHECKS
	if (argc != 2) {
		perror("Too many or too little arguments.");
		return 1;
	}
	#endif

	FILE* file = fopen(argv[1], "r");

	#ifdef CHECKS
	if(file == NULL) {
		perror("Can't open file.");
		return 1;
	}
	#endif

	clock_t startTime = clock();
	
	while (1) {
		filled = fread((uint8_t*) buffer, 1, BUFFER_SIZE_U8, file);
		if(filled == 0) {
			break;
		}
		read += filled;
		count_ones();
	}

	clock_t endTime = clock();

	fclose(file);

	ULL total_bits = read*8;
	ULL ones = count;
	ULL zeroes = total_bits - ones;

	double timeelapsed = (double)(endTime - startTime) / (double)(CLOCKS_PER_SEC / 1000);

	printf("Found %lld ones (1) and %lld zeroes (0) in %lld bits total.\n", ones, zeroes, total_bits);

	printf("It took %.3f ms.\n", timeelapsed);

	return 0;
}
