#include "stdio.h"
#include "math.h"

char is_prime(int a) {
	int lim = (int) sqrt(a);
	printf("-%d\n", lim);
	for (int i = 2; i <= lim; i++) {
		for (int j = 2; j < i; j++) {
			if(i % j == 0) {
				return 0;
			}
		}
	}
	return 1;
}

int main(int argc, char** argv) {
	int lim;
	scanf("%d", &lim);
	for (int i = 1; i <= lim; i++) {
		if (is_prime(i)) {
			printf("%d\n", i);
		}
	}
	return 0;
}
