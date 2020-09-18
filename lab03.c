#include "stdio.h"
#include "string.h"

int main(int argc, char** argv) {
	int pos;
	scanf("%d", &pos);
	int step = 10;
	int len = 1;
	while(1) {
		if (pos < step) {
			printf("%d");
		}
	}
	return 0;
}
