#include "stdio.h"

// Выводит n-ую цифру в последовательности натуральных чисел (Счёт идёт он 0, то есть 0->1, 1->2, 9->1, 10->0)
char digit(int pos) {
	int seq_len = 9; // Количество чисел в цепочке порядка
	int num_len = 1; // Количество цифр в числах порядка
	int seq_off = 1; // Минимальное число в цепи порядка
	while(1) {
		if (pos < num_len * seq_len) { // Если указатель находится в пределах данного порядка
			int number = seq_off + pos / num_len; // Находим число на данной позиции
			int digit  = pos % num_len; // Находим цифру в данном числе
			for(int i = num_len - digit - 1; i != 0; i--) { // Отбрасываем ненужные цифры в числе справа
				number /= 10;
			}
			return number % 10; // Выводим одну цифру справа
		} else { // Если указатель выходит за пределы данного порядка
			pos -= seq_len * num_len; // Смещаем указатель на количество цифр в порядке
			num_len++; // Увеличиваем количество цифр в числах порядка
			seq_len *= 10; // Увеличиваем количество чисел в цепочке порядка
			seq_off *= 10; // Увеличиваем минимальное число в цепи порядка
		}
	}
}

// === Функции для отладки ===
// Можно вызвать debug() для демонстрации того, что все числа выводятся правильно

char debug_seq(int *a, int digits, int start, int end) {
	for(int i = start; i != end; i++) {
		for(int d = 0; d != digits; d++) {
			printf("%d", digit((*a)++));
		}
		printf(" ");
	}
	printf("\n");
}


void debug() {
	printf("Debug sequence:\n");
	int a = 0;
	debug_seq(&a, 1,   0,    9); // Числа от    1 до    9
	debug_seq(&a, 2,   9,   99); // Числа от   10 до   99
	debug_seq(&a, 3,  99,  999); // Числа от  100 до  999
	debug_seq(&a, 4, 999, 9999); // Числа от 1000 до 9999
}

// ===========================

int main(int argc, char** argv) {
	printf("Enter digit's position to be printed:\n");

	int pos;
	scanf("%d", &pos);

	printf("The digit at %d place in a sequence is %d.\n", pos, digit(pos));

	return 0;
}
