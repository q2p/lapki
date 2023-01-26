#include "stdio.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define SIZE 30

char** EN_MAP[25] = {
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

char*** RU_MAP = {
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

char eng_to_codes(char** input, char map_length, char*** MAP) {
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

void generateKeyTable(char key[], int ks, char keyT[25]){
	int i, j, k, flag = 0;

	// a 26 character hashmap
	// to store count of the alphabet
	char* dicty = (char*)calloc(26, sizeof(char));
	for (i = 0; i < ks; i++){
		if (key[i] != 'J'){
			dicty[key[i] - 'A'] = 2;
		}
	}
	dicty['J' - 'A'] = 1;

	char cell = 0;
	for(char idx = 0; cell != 25 && idx !=26; idx++) {
		if (dicty[key[idx] - 'A'] == 2){
			dicty[key[idx] - 'A'] = 1;
			keyT[cell] = key[idx];
			cell++;
		}
	}

	for(char idx = 0; cell != 25 && idx !=26; idx++) {
		if (dicty[idx] == 0){
			keyT[cell] = idx + 'A';
			cell++;
		}
	}
}

// Function to convert the string to uppercase
void toUpperCase(char plain[], int ps){
	for (int i = 0; i < ps; i++){
		if (plain[i] >= 'a' && plain[i] <= 'z'){
			plain[i] -= 'a'-'A';
		}
	}
}
 
// Function to remove all spaces in a string
int removeSpaces(char* plain, int ps){
		int i, count = 0;
		for (i = 0; i < ps; i++)
				if (plain[i] != ' ')
						plain[count++] = plain[i];
		plain[count] = '\0';
		return count;
}

// Function to search for the characters of a digraph
// in the key square and return their position
void search(char keyT[25], char a, char b, int arr[])
{
		int i, j;
 
		if (a == 'j')
				a = 'i';
		else if (b == 'j')
				b = 'i';
 
		for (i = 0; i < 5; i++) {
 
				for (j = 0; j < 5; j++) {
 
						if (keyT[i][j] == a) {
								arr[0] = i;
								arr[1] = j;
						}
						else if (keyT[i][j] == b) {
								arr[2] = i;
								arr[3] = j;
						}
				}
		}
}

// Function to find the modulus with 5
int mod5(int a)
{
		return (a % 5);
}
 
// Function to make the plain text length to be even
int prepare(char str[], int ptrs)
{
		if (ptrs % 2 != 0) {
				str[ptrs++] = 'z';
				str[ptrs] = '\0';
		}
		return ptrs;
}

// Function for performing the encryption
void encrypt(char str[], char keyT[25], int ps)
{
		int i, a[4];
 
		for (i = 0; i < ps; i += 2) {
 
				search(keyT, str[i], str[i + 1], a);
 
				if (a[0] == a[2]) {
						str[i] = keyT[a[0]][mod5(a[1] + 1)];
						str[i + 1] = keyT[a[0]][mod5(a[3] + 1)];
				}
				else if (a[1] == a[3]) {
						str[i] = keyT[mod5(a[0] + 1)][a[1]];
						str[i + 1] = keyT[mod5(a[2] + 1)][a[1]];
				}
				else {
						str[i] = keyT[a[0]][a[3]];
						str[i + 1] = keyT[a[2]][a[1]];
				}
		}
}

void printTable(char keyT[5][5]){
	printf("\n");
	for(int i=0;i<5;i++){
		for(int j=0;j<5;j++){
			printf("%c ", keyT[i][j]);
		}
		printf("\n");
	}
}

void encryptByPlayfairCipher(char str[], char key[]){
	char ps, ks, keyT[5][5];

	// Key
	ks = strlen(key);
	ks = removeSpaces(key, ks);
	toUpperCase(key, ks);
 
	// Plaintext
	ps = strlen(str);
	ps = removeSpaces(str, ps);
	toUpperCase(str, ps);
 
	ps = prepare(str, ps);
 
	generateKeyTable(key, ks, keyT);
 
	printTable(keyT);
	encrypt(str, keyT, ps);
}

int main(int argv, char** argc){
	char str[SIZE], key[SIZE];
 
	// Key to be encrypted
	strcpy(key, "Monarchy");
	printf("Key text: %s\n", key);
 
	// Plaintext to be encrypted
	strcpy(str, "instruments");
	printf("Plain text: %s\n", str);

	// encrypt using Playfair Cipher
	encryptByPlayfairCipher(str, key);
 
	printf("Cipher text: %s\n", str);
 
	return 0;
}