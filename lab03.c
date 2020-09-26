#include "stdio.h"

// Проверяет, является ли число простым
char is_prime(int number) {
	// Проверять значения больше sqrt(number) нам не нужно, по свойству умножения
	// i*i <= number, равносильно i <= sqrt(number)
	for (int i = 2; i*i <= number; i++) {
		if(number % i == 0) {
			return 0;
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
