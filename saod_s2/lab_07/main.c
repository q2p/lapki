#include <stdio.h>
#include <string.h>
#include <stdint.h>

enum {
	MAX_LENGTH = 128,
};

size_t length = 0;
uint8_t elements[MAX_LENGTH];
uint8_t current[MAX_LENGTH];
uint8_t best[MAX_LENGTH];
uint32_t sums[2] = { 0, 0 };
uint32_t smallest_diff = UINT32_MAX;
 
void find_smallest_diff(size_t idx) {
	if (idx == length) {
		int32_t diff = abs((int32_t)sums[0] - (int32_t)sums[1]);
		if (diff < smallest_diff) {
			printf("sd %d\n", diff);
			smallest_diff = diff;
			memcpy(best, current, MAX_LENGTH);
		}
	} else {
		for (uint8_t flip = 0; flip != 2; flip++) {
			current[idx] = flip;
			sums[flip] += elements[idx];
			find_smallest_diff(idx+1);
			sums[flip] -= elements[idx];
		}
	}
};

int main() {
	while (length != MAX_LENGTH) {
		int v;
		scanf("%d", &v);
		if (v == 0) {
			break;
		}
		elements[length++] = v;
	}
	
	find_smallest_diff(0);
	
	printf("Smallest difference: %d\n", smallest_diff);
	printf("Best set mask: ");
	for (size_t i = 0; i != length; i++) {
		printf("%d", best[i]);
	}
	printf("\nBest set indicies:");
	for (size_t i = 0; i != length; i++) {
		if (best[i]) {
			printf(" %d", i);
		}
	}
	printf("\n");
	
	return 0;
}