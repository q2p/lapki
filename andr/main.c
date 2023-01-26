#include "stdio.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

enum {
	EN_MAP_SIZE = 26,
	RU_MAP_SIZE = 33,
};
char *EN_MAP[EN_MAP_SIZE][2] = {
	{"a", "A"},
	{"b", "B"},
	{"c", "C"},
	{"d", "D"},
	{"e", "E"},
	{"f", "F"},
	{"g", "G"},
	{"h", "H"},
	{"i", "I"},
	{"j", "J"},
	{"k", "K"},
	{"l", "L"},
	{"m", "M"},
	{"n", "N"},
	{"o", "O"},
	{"p", "P"},
	{"q", "Q"},
	{"r", "R"},
	{"s", "S"},
	{"t", "T"},
	{"u", "U"},
	{"v", "V"},
	{"w", "W"},
	{"x", "X"},
	{"y", "Y"},
	{"z", "Z"}
};

char *RU_MAP[RU_MAP_SIZE][2] = {
	{"а", "А"},
	{"б", "Б"},
	{"в", "В"},
	{"г", "Г"},
	{"д", "Д"},
	{"е", "Е"},
	{"ё", "Ё"},
	{"ж", "Ж"},
	{"з", "З"},
	{"и", "И"},
	{"й", "Й"},
	{"к", "К"},
	{"л", "Л"},
	{"м", "М"},
	{"н", "Н"},
	{"о", "О"},
	{"п", "П"},
	{"р", "Р"},
	{"с", "С"},
	{"т", "Т"},
	{"у", "У"},
	{"ф", "Ф"},
	{"х", "Х"},
	{"ц", "Ц"},
	{"ч", "Ч"},
	{"ш", "Ш"},
	{"х", "Щ"},
	{"ъ", "Ъ"},
	{"ы", "Ы"},
	{"ь", "Ь"},
	{"э", "Э"},
	{"ю", "Ю"},
	{"я", "Я"}
};

char utf_to_num(char** input, char map_length, char *MAP[][2]) {
	for(char i = 0; i != map_length; i++) {
		for(char j = 0; j != 2; j++) {
			char* prefix = MAP[i][j];
			size_t prefix_len = strlen(prefix);
			size_t str_len = strlen(*input);
			if (str_len >= prefix_len && memcmp(*input, prefix, prefix_len) == 0) {
				*input += prefix_len;
				return i;
			}
		}
	}
	return -1;
}

char* num_to_utf(char num, char *MAP[][2]) {
	return MAP[num][1];
}

int main(int argv, char** argc){
	char* s1 = "bablo";
	char* s2 = "азбука";

	while(*s1 != '\0') {
		char code = utf_to_num(&s1, EN_MAP_SIZE, EN_MAP);
		char* utf = num_to_utf(code, EN_MAP);
		printf("%s", utf);
	}
	printf("\n---\n");
	while(*s2 != '\0') {
		char code = utf_to_num(&s2, RU_MAP_SIZE, RU_MAP);
		char* utf = num_to_utf(code, RU_MAP);
		printf("%s", utf);
	}
	printf("\n");
 
	return 0;
}