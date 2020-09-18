#include "stdio.h"
#include "string.h"

int main(int argc, char** argv) {
	char str[20];
	scanf("%s", str);
	int l = strlen(str);
	for (int i = 0; i < l / 2; i++) {
		char t = str[i];
		str[i] = str[l-i-1];
		str[l-i-1] = t;
	}
	printf("%s", str);
	return 0;
}
