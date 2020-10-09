#include <string.h>
#include <stdio.h>

int main(int argc, char** argv) {
	// Создаём таблицу, в которой мы храним: присутсвует ли i-ый символ в множестве (A-B)
	// Это лучше чем перебор всех символов во вложенном цикле
	// Сложность:
	//   Перебор символов через два цикла: O(n1 * n2)
	//   Таблица:                          O(n1 + n2 + c)
	char table[256] = {0};

	char str1[80];
	char str2[80];
	scanf("%79s", str1); // Читаем только 79 символов
	scanf("%79s", str2); // Читаем только 79 символов

	char len = strlen(str1);
	for(char i = 0; i != len; i++) {
		table[str1[i]] = 1; // Исключаем i-ый элемент первого множества
	}
	len = strlen(str2);
	for(char i = 0; i != len; i++) {
		table[str2[i]] = 0; // Исключаем i-ый элемент второго множества
	}

	for(short i = 0; i != 256; i++) {
		if(table[i]) {
			printf("%c", i);
		}
	}
	printf("\n");
	return 0;
}
