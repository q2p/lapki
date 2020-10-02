#include "stdio.h"
#include "stdlib.h"

// Наибольший общий делитель
int gcd(int v1, int v2) {
	while(v1 != 0) {
		int tmp = v1;
		v1 = v2 % v1;
		v2 = tmp;
	}

	return v2;
}

// Наименьшее общее кратное
int lcm(int v1, int v2) {
	// Чтобы правильно находить НОК для отрицательных чисел,
	// нужно, аргументы сделать положительными.
	v1 = abs(v1);
	v2 = abs(v2);

	// Нахождение НОК через НОД
	return v1 / gcd(v1, v2) * v2;
}

int main(int argc, char** argv) {
	int v1;
	int v2;

	scanf("%d", &v1);
	scanf("%d", &v2);

	printf("Least common multiple of %d and %d is ", v1, v2);
	// Не существует числа, которое можно поделить на ноль.
	if (v1 == 0 || v2 == 0) {
		printf("not defined.\n");
	} else {
		printf("%d.\n", lcm(v1, v2));
	}

	return 0;
}
