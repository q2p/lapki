#include "stdio.h"
#include "stdlib.h"

// Наибольший общий делитель
int gcd(int v1, int v2) {
	// Чтобы правильно находить НОД для отрицательных чисел,
	// нужно, аргументы сделать положительными.
	v1 = abs(v1);
	v2 = abs(v2);

	while(v1 != 0) {
		int tmp = v1;
		v1 = v2 % v1;
		v2 = tmp;
	}

	return v2;
}

int main(int argc, char** argv) {
	int v1;
	int v2;

	scanf("%d", &v1);
	scanf("%d", &v2);

	printf("Highest common factor of %d and %d is ", v1, v2);
	// Для двух нулей наибольшим общим делителем является бесконечно большое число.
	if (v1 == 0 && v2 == 0) {
		printf("not defined.\n");
	} else {
		printf("%d.\n", gcd(v1, v2));
	}

	return 0;
}
