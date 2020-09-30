#include "stdio.h"

#define N 128 // Размер массива
#define N2 (N*N) // Размер массива в квадрате

void exclude_non_prime(char *sequence) {
	int K = 2;
	// K*K <= N*N равносильно K <= sqrt(N)
	while(K*K <= N2) {
		while(1) {
			if (K == N) { // Если дошли до конца массива
				return;
			}
			if (sequence[K] != 0) { // Если нашли потенциально простое число
				break;
			}
			K++; // Перешагиваем к следующему числу
		}
		// Перебираем числа, делящиеся на K
		for (int m = K*2; m < N; m += K) {
			sequence[m] = 0; // Помечаем число как не простое
		}
		K++; // Перешагиваем к следующему числу
	}
}

int main(int argc, char** argv) {
	// 1 - число простое, 0 - число не простое.
	char sequence[N];

	// Счёт начинаем с нуля, для упрощения вычислений. Ноль не является простым числом.
	sequence[0] = 0;
	// Временно помечаем все остальные числа как простые
	for(int i = 1; i != N; i++) {
		sequence[i] = 1;
	}

	// Помечаем числа, которые простыми не являются
	exclude_non_prime(sequence);

	for(int i = 0; i != N; i++) {
		if (sequence[i]) { // Если число простое
			printf("%d ", i);
		}
	}

	printf("\n");

	return 0;
}
