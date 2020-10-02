#include "stdio.h"
#include "stdlib.h"

#define N (128)

int main(int argc, char** argv) {
	float array[N];

	int len;

	scanf("%d", &len);

	if (len > N) {
		printf("Length is too big.");
		return 1;
	}

	for (int i = 0; i != len; i++) {
		array[i] = (float) rand() / (float) RAND_MAX;
	}

	printf("Before sort:\n");
	for (int i = 0; i != len; i++) {
		printf("%.3f ", array[i]);
	}

	char swapped;
	do {
		swapped = 0;
		for (int i = 1; i != len; i++) {
			float* v1 = &array[i-1];
			float* v2 = &array[i  ];
			float temp;
			if (*v2 < *v1) {
				temp = *v1;
				*v1 = *v2;
				*v2 = temp;
				swapped = 1;
			}
		}
	} while(swapped);

	printf("\nAfter sort:\n");
	for (int i = 0; i != len; i++) {
		printf("%.3f ", array[i]);
	}
	printf("\n");

	return 0;
}
