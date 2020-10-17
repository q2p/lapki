#include <string.h>
#include <stdio.h>

int fraction_length_max;

// Входное число
char source[64];
char sp = 0;
// Исходное основание системы
int base_form;

// Выводимый результат
char target[64];
char tp = 0;
// Желаемое основание системы
int base_to;

const char digits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
unsigned char table[256];
void initialize_lookup_table() {
	for(short i = 0; i != 256; i++) {
		table[i] = 0xFF;
	}
	for(char i = 0; i != 10; i++) {
		table['0'+i] = i;
	}
	for(char i = 0; i != 6; i++) {
		table['a'+i] = 10+i;
		table['A'+i] = 10+i;
	}
}

void swap(char* c1, char* c2) {
	*c1 ^= *c2;
	*c2 ^= *c1;
	*c1 ^= *c2;
}

void reverse_string(char* string, unsigned char length) {
	unsigned char to = length / 2;
	for(unsigned char i = 0; i != to; i++) {
		swap(&string[i], &string[length - i - 1]);
	}
}

// Функция записи целой части
void integer_to_string(unsigned long long integer) {
	do {
		// Записываем младшую цифру
		char rem = integer % base_to;
		target[tp] = digits[rem];
		tp++;

		// Сдвигаем число
		integer /= base_to;
	} while (integer != 0);

	// Разворачиваем строку, чтобы младная цифра была в конце строки
	reverse_string(target, tp);
}

// Функция обработки дробной части
char append_fractional() {
	// Множитель для отделения дробной части от целой
	int multiplier = 1;
	int temp = 0;
	while(1) {
		char c = source[sp];
		sp++;

		if (c == '\0') {
			break;
		}

		unsigned char value = table[(unsigned char) c];

		// Если цифра выходит за пределы основания или введён недопустимый символ
		if (value >= base_form) {
			printf("Error: Digit %c is not allowed.\n", c);
			return 1;
		}

		temp = temp * base_form + value;
		multiplier *= base_form;
	}

	// Если дробная часть = 0
	if (temp == 0) {
		return 0;
	}

	// Добавляем точку
	target[tp] = '.';
	tp++;

	// Сколько знаков после запятой записывать
	int limit = sizeof(target) - 1;
	if (limit > tp + fraction_length_max) {
		limit = tp + fraction_length_max;
	}

	// Пока не дошли до допустимого количества знаков после запятой
	while(tp < limit) {
		temp *= base_to;
		char decimal = temp / multiplier;
		temp %= multiplier;

		target[tp] = digits[decimal];
		tp++;

		// Если дробная часть кончилась
		if(temp == 0) {
			break;
		}
	}

	return 0;
}

char read_input() {
	printf("Please enter: Initial base, Target base, Length of fractional part, Number in initial base...\n");

	scanf("%d%d%d%63s", &base_form, &base_to, &fraction_length_max, source);

	if (base_form < 2 || base_form > 16) {
		printf("Error: Initial base must be in range of [2..16].\n");
		return 1;
	}
	if (base_to < 2 || base_to > 16) {
		printf("Error: Target base must be in range of [2..16].\n");
		return 1;
	}
	if (fraction_length_max < 1 || base_to > 10) {
		printf("Error: Fraction length must be in range of [1..10].\n");
		return 1;
	}

	return 0;
}

int main(int argc, char** argv) {
	// Инициализируем таблицу для конверсии систем счисления
	initialize_lookup_table();

	// Получаем ввод пользователя
	char error = read_input();
	// Если произошла ошибка
	if (error) {
		return error;
	}

	// Переменная для хранения целой части
	unsigned long long integer = 0;

	while(1) {
		char c = source[sp];
		sp++;

		if (c == '\0') { // Конец строки
			// Записываем целое число
			integer_to_string(integer);
			break;
		} else if (c == '.') { // Конец целой части
			// Записываем целую часть
			integer_to_string(integer);
			// Обрабатываем дробную часть
			char error = append_fractional();
			// Если произошла ошибка
			if (error) {
				return 1;
			}
			break;
		}

		// Конвертируем цифру из символа в число
		unsigned char value = table[(unsigned char) c];

		// Если цифра выходит за пределы основания или введён недопустимый символ
		if (value >= base_form) {
			printf("Error: Digit %c is not allowed.\n", c);
			return 1;
		}

		// Прибавляем к числу
		integer = integer * base_form + value;
	}

	// Закрываем получившуюся строку
	target[tp] = '\0';
	// Выводим результат
	printf("%s\n", target);

	return 0;
}
