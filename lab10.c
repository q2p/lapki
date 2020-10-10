#include <string.h>
#include <stdio.h>

int main(int argc, char** argv) {
	// Создаём таблицу, в которой мы храним: присутсвует ли i-ый символ во втором множестве
	//
	// Данный подход лучше чем перебор всех символов во вложенном цикле
	// Сложность:
	//   Перебор символов через два цикла: O(n1 * n2)
	//   Таблица:                          O(n1 + n2)
	//   Где:
	//      n1 - длина первого множества
	//      n2 - длина второго множество
	char table[256] = {0};

	char str1[80];
	char str2[80];
	// Читаем только 79 символов
	scanf("%79s%79s", str1, str2);

	int len = strlen(str2);
	for(int i = 0; i != len; i++) {
		// Помечаем i-ый элемент второго множества как присутсвующий
		table[(unsigned char) str2[i]] = 1;
	}

	len = strlen(str1);
	for(int i = 0; i != len; i++) {
		char c = str1[i];

		// Если i-ый символ отсутсвует в множестве B, то выводим его
		if(!table[(unsigned char) c]) {
			printf("%c", c);

			// Строку снизу можно раскоментировать, если мы не хотим выводить повторяющиеся символы
			// table[(unsigned char) c] = 1;
		}
	}

	printf("\n");
	
	return 0;
}
