#include "stdio.h"
#include "string.h"
#include "locale.h"
#include "stdlib.h"
#include "malloc.h"

int checkNoLetter(char* str, char letter){    //проверка отсутствия символа в строке
	for (int i = 0; i < strlen(str); i++){
		if (letter == str[i]){
			return 0;
		}
	}
	return 1;
}

char* openFile(char* placeOfFile){
	char* str = (char*)malloc(sizeof(char) * 1);
	str[0] = '\0';
	FILE *cezfl;
	cezfl = fopen(placeOfFile, "r");
	if (cezfl == NULL){
		printf("Error, programm can't find a file!\n");
	}
	int buff_size = 1;
	while(1){
		char tempstr[20];
		char* read_result = fgets(tempstr, 20, cezfl);
		if(read_result == NULL){
			break;
		}
		buff_size += strlen(tempstr);
		str = (char*)realloc(str, sizeof(char) * buff_size);
		strcat(str, tempstr);
  }
	return str;
}

char* createSpecAlphabet(int n, char* key, char firstLetterOfAlphabet, char amountletters){
	n = amountletters - n;
	while(n > amountletters){
		n - amountletters;
	}
	while(n < 0){
		n + amountletters;
	}
	char* specAlphabet = (char*)malloc(sizeof(char) * amountletters);                   //создаю закодированный алфавит	
	int letterOfAlpabet = 0;
	int specLetterOfAlpabet = 0;
	int y;
	if (strlen(key) == 0){             //если нет ключа
		y = 0;     //подгоняю шифр цезаря под общие стандарты без ключа
	} else {
		y = 1;
	}
	for (; specLetterOfAlpabet < n-y; letterOfAlpabet++){             //алфавит до ключа
		if(checkNoLetter(key, firstLetterOfAlphabet + amountletters - n + letterOfAlpabet)){
			specAlphabet[specLetterOfAlpabet] = firstLetterOfAlphabet + amountletters - n + letterOfAlpabet;
			specLetterOfAlpabet++;
		}
	}
	int lastEnd = specLetterOfAlpabet;
	for(int i = 0; specLetterOfAlpabet < lastEnd + strlen(key); specLetterOfAlpabet++, i++){
		specAlphabet[specLetterOfAlpabet] = key[i];                     //вставляю ключ в алфавит
	}
	for (char i = 0; specLetterOfAlpabet < amountletters; i++){                  //алфавит после ключа
		if(checkNoLetter(key, firstLetterOfAlphabet + i)){
			specAlphabet[specLetterOfAlpabet] = firstLetterOfAlphabet + i;
			specLetterOfAlpabet++;
		}
	}
	return specAlphabet;
}

char* keyCezar(char* str, int n, char* key, char Asmall, char Abig, char amountletters){
	char* specAlphabet = createSpecAlphabet(n, key, Asmall, amountletters);
	char* bigKEY = (char*)malloc(sizeof(char) * strlen(key));            //создаю алфавит заглавных букв
	bigKEY[0] = '\0';
	for(int i = 0; i < strlen(key); i++){
		bigKEY[i] = key[i] - (Asmall-Abig);
	}
	char* specBIGALBHABET = createSpecAlphabet(n, bigKEY, Abig, amountletters);
	
	char* specStr = (char*)malloc(sizeof(char) * (strlen(str) + 1)); 
	specStr[strlen(str)] = '\0';	
	char numberOfLetter;         //кодирую слово
	for(int i = 0; i < strlen(str); i++){
		if((str[i] >= Abig) && (str[i] < Abig + amountletters)){
			numberOfLetter = str[i] - Abig;       //нахожу номер буквы и подставляю этот номер в закодированный алфавит
			specStr[i] = specBIGALBHABET[numberOfLetter];
		} else if((str[i] >= Asmall) && (str[i] < Asmall + amountletters)){
			numberOfLetter = str[i] - Asmall;
			specStr[i] = specAlphabet[numberOfLetter];
		} else {
			specStr[i] = str[i];
		}
	}
	free(bigKEY);
	free(specAlphabet);
	free(specBIGALBHABET);
	return specStr;
}

char* deKeyCezar(char* str, int n, char* key, char Asmall, char Abig, char amountletters){
	char* specAlphabet = createSpecAlphabet(n, key, Asmall, amountletters);
	char* bigKEY = (char*)malloc(sizeof(char) * strlen(key));            //создаю алфавит обычных и заглавных букв
	for(int i = 0; i < strlen(key); i++){
		bigKEY[i] = key[i] - (Asmall - Abig);
	}
	char* specBIGALBHABET = createSpecAlphabet(n, bigKEY, Abig, amountletters);

	char* specStr = (char*)malloc(sizeof(char) * (strlen(str) + 1));
	specStr[strlen(str)] = '\0'; 
	char numberOfLetter;
	for(int i = 0; i < strlen(str); i++){            //разкодирую слово
		if((str[i] >= Abig) && (str[i] < Abig + amountletters)){
			char j = 0;
			for(; (str[i] != specBIGALBHABET[j]) && (j < amountletters - 1); j++){
			}
			specStr[i] = Abig + j;
		} else if((str[i] >= Asmall) && (str[i] < Asmall + amountletters)){
			char j = 0;
			for(; (str[i] != specAlphabet[j]) && (j < amountletters - 1); j++){
			}
			specStr[i] = Asmall + j;
		} else {
			specStr[i] = str[i];
		}
	}
	free(bigKEY);
	free(specAlphabet);
	free(specBIGALBHABET);
	return specStr;
}

char* keyCezarFile(char* placeOfFile, int n, char* key, char Asmall, char Abig, char amountletters, char decrypt){
  char* str = openFile(placeOfFile);
	char* specStr;
	if(decrypt){
		specStr = deKeyCezar(str, n, key, Asmall, Abig, amountletters);
	} else {
		specStr = keyCezar(str, n, key, Asmall, Abig, amountletters);
	}
	free(str);
	return specStr;
}

void freeKeyCezar(char* str){
	free(str);
}

void playKeyCezar(char* str1file, char* str2file){
	char lang;
	printf("\nSet language: 0 for English, 1 for Russian(Win1251), 2 for custom\n");
	scanf("%d", &lang);
	char Asmall;
	char Abig;
	char letters_amount;
	if(lang == 0){
		Asmall = 'a';
		Abig = 'A';
		letters_amount = 26;
	} else if(lang == 1){
		Asmall = '�';
		Abig = '�';
		letters_amount = 32;
	} else if(lang == 2){
		printf("\nWrite 3 things: first letter of alphabet in lowercase, in capital size and amount of letters in alphabet\n");
	}

	char str1[100];
	char str2[100];	
	char* str1d;	
	char* str2d;	
	char* key = (char*)malloc(sizeof(char) * letters_amount);
	char* coded1;
	char* coded2;
	int n;
	char coding;
	char cont = 1;
	while(cont){
		char oper;
		printf("\nSet operating mode: 0 for write text manually, 1 for using files\n");
		scanf("%d", &oper);
		if(oper == 0){
			printf("\n0 for encoding, 1 for decoding\n");
			scanf("%d", &coding);
			fgets(str1, 2, stdin);
			printf("\nWrite your text here:\n");
			fgets(str1, 100, stdin);
			printf("\nWrite your key here (must be written in lowercase with different letters):\n");
			scanf("%s", key);
			printf("\nWrite your offset here:\n");
			scanf("%d", &n);
			if(coding == 0){
				coded1 = keyCezar(str1, n, key, Asmall, Abig, letters_amount);
			} else if(coding){
				coded1 = deKeyCezar(str1, n, key, Asmall, Abig, letters_amount);
			}
			printf("\n_______________________________________________________________________________\n\n");
			puts(coded1);
			printf("_______________________________________________________________________________\n\n");
		} else {
			fgets(str1, 2, stdin);
			printf("\nWrite your key here:\n");
			scanf("%s", key);
			printf("\nWrite your offset here:\n");
			scanf("%d", &n);

			str1d = openFile(str1file);
			str2d = openFile(str2file);
			coded1 = keyCezar(str1d, n, key, Asmall, Abig, letters_amount);
			coded2 = deKeyCezar(str2d, n, key, Asmall, Abig, letters_amount);

			printf("\n_______________________________________________________________________________\n\n");
			printf("Original");
			printf("\n_______________________________________________________________________________\n\n");
			puts(str1d);
			printf("_______________________________________________________________________________\n\n");
			puts(str2d);
			printf("_______________________________________________________________________________\n\n");

			printf("Modified");
			printf("\n_______________________________________________________________________________\n\n");
			puts(coded1);
			printf("_______________________________________________________________________________\n\n");
			puts(coded2);
			printf("_______________________________________________________________________________\n\n");
		}
		printf("\nPut 0 to quit or 1 to continue coding\n");
		scanf("%d", &cont);
	}
	free(str1d);
	free(str2d);
	free(coded1);
	free(coded2);
	free(key);
}

int main(int argc, char** argv){
	//playKeyCezar(argv[1], argv[2]);
  char* str = keyCezarFile(argv[1], 24, "����", '�', '�', 32, 1);
	puts(str);
	return 0;
}

/*
qwertyuiopasdfghjklzxcvbnm
abcdefghijklmnopqrstuvwxyz

The quick brown fox jumps over the lazy dog.

 gcc ./s2prgrm_krs.c -o ./s2prgrm_krs.exe
 s2prgrm_krs.exe D:\er\prgrmng\S1_ru.txt 

gcc -c -fpic foo.c
gcc -shared -o libfoo.dll foo.o
gcc -o test main.c -lfoo
(if in another place)gcc -L/home/username/foo -Wall -o test main.c -lfoo
*/