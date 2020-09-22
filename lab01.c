#include "stdio.h"
#include "string.h"

int main(int argc, char** argv) {
	char string[32];
	scanf("%s", string);
	int l = strlen(string);
	for (int i = 0; i < l / 2; i++) {
		// Зеркально переставльяем символы.
		// Подобный подход работает одновременно с целыми и дробными числами.
		char temp = string[i];
		string[i] = string[l-i-1];
		string[l-i-1] = temp;
	}
	printf("%s", string);
	return 0;
}
