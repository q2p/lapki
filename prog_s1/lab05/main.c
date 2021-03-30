#include "stdio.h"

#define N (16*1024) // Размер массива

void exclude_non_prime(char *sequence, int length) {
	int K = 2;
	int lengthSq = length*length;
	// K*K <= N*N равносильно K <= sqrt(N)
	while(K*K <= lengthSq) {
		while(1) {
			if (K == length) { // Если дошли до конца последовательности
				return;
			}
			if (sequence[K] != 0) { // Если нашли потенциально простое число
				break;
			}
			K++; // Перешагиваем к следующему числу
		}
		// Перебираем числа, делящиеся на K
		for (int m = K*2; m < length; m += K) {
			sequence[m] = 0; // Помечаем число как не простое
		}
		K++; // Перешагиваем к следующему числу
	}
}

int main(int argc, char** argv) {
	// 1 - число простое, 0 - число не простое.
	char sequence[N];

	int length;
	scanf("%d", &length);

	if(length > N) {
		printf("The number is too big.")
	}

	// Счёт начинаем с нуля, для упрощения вычислений. Ноль не является простым числом.
	sequence[0] = 0;
	// Временно помечаем все остальные числа как простые
	for(int i = 1; i < length; i++) {
		sequence[i] = 1;
	}

	// Помечаем числа, которые простыми не являются
	exclude_non_prime(sequence, length);

	for(int i = 0; i < length; i++) {
		if (sequence[i]) { // Если число простое
			printf("%d ", i);
		}
	}

	printf("\n");

	return 0;
}
