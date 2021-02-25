// gcc -O3 ./main.c && ./a.out

#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"

void fill_with_ones(uint8_t *sequence, size_t length, size_t idx) {
	if (idx < length) {
		sequence[idx] = 1;
		fill_with_ones(sequence, length, idx+1);
	}
}

void exclude_multiples(uint8_t *sequence, size_t length, size_t idx, size_t shift) {
	if (idx < length) {
		sequence[idx] = 0;
		exclude_multiples(sequence, length, idx + shift, shift);
	}
}

void check_if_prime(uint8_t *sequence, size_t length, size_t idx) {
	if (idx < length) {
		if (sequence[idx]) {
			exclude_multiples(sequence, length, idx*2, idx);
			printf(" %ld", idx);
		}
		check_if_prime(sequence, length, idx+1);
	}
}

void print_primes(int max_number) {
	size_t length = max_number + 1;
	uint8_t *sequence = (uint8_t*) malloc(length);
	fill_with_ones(sequence, length, 0);
	printf("1");
	check_if_prime(sequence, length, 2);
	printf("\n");
	free(sequence);
}

int main(int argc, char** argv) {
	int max_number;
	scanf("%d", &max_number);
	print_primes(max_number);
	return 0;
}
