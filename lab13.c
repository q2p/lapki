#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct __STRING {
	char* ptr;
	size_t len;
	size_t capacity;
} String;

String vec_new(size_t starting_capacity) {
	if (starting_capacity == 0) {
		starting_capacity = 1;
	}
	String s;
	s.ptr = (char*) malloc(starting_capacity);
	s.len = 1;
	s.capacity = starting_capacity;
	*s.ptr = '\0';
	return s;
}

void string_free(String* vec) {
	free(vec->ptr);
}

void string_reserve(String* vec, size_t additional) {
	size_t req = vec->len+additional;
	if (req > vec->capacity) {
		vec->capacity = req + req / 2;
		vec->ptr = realloc(vec->ptr, vec->capacity);
	}
}

void string_push(String* vec, char value) {
	string_reserve(vec, 1);
	vec->ptr[vec->len-1] = value;
	vec->ptr[vec->len  ] = '\0';
	vec->len++;
}

void string_concat(String* vec, char* append) {
	size_t sl = strlen(append);
	string_reserve(vec, sl);
	memcpy(vec->ptr + vec->len - 1, append, sl + 1);
	vec->len = vec->len + sl;
}

char* string_idx(String* vec, size_t idx) {
	if (idx >= vec->len - 1) {
		return NULL;
	} else {
		return &vec->ptr[idx];
	}
}

char* string_get_str(String* vec) {
	return vec->ptr;
}

void string_push_int(String* vec, unsigned int value) {
	char old_len = vec->len;
	do {
		string_push(vec, '0' + (value % 10));
		value /= 10;
	} while (value != 0);

	char* l = vec->ptr + old_len - 1;
	char* r = vec->ptr + vec->len - 2;

	while (l < r) {
		*l ^= *r;
		*r ^= *l;
		*l ^= *r;
		l++;
		r--;
	}
}

void string_shrink(String* vec) {
	vec->capacity = vec->len;
	vec->ptr = realloc(vec->ptr, vec->capacity);
}

int fibonacci(int number) {
	int num1 = 0;
	int num2 = 1;
	int next = 1;
 
	for (int i = 0 ; i < number-1 ; ++i) {
		next = num1 + num2;
		num1 = num2;
		num2 = next;
	}
	return next;
}

int fibbonacci_prev = 1;
int fibbonacci_curr = 1;

void fibbonacci_push(String* str, int how_much) {
	while (how_much) {
		string_push_int(str, fibbonacci_curr);

		int next = fibbonacci_prev + fibbonacci_curr;
		fibbonacci_prev = fibbonacci_curr;
		fibbonacci_curr = next;

		how_much--;
	}
}

unsigned long long gcd(unsigned long long n1, unsigned long long n2) {
	while(n1 != n2) {
		if(n1 > n2)
			n1 -= n2;
		else
			n2 -= n1;
	}

	return n1;
}

int main(int argc, char** argv) {
	while (1) {
		int acc;
		scanf("%i", &acc);
		double pi = 3;
		unsigned int c = 2;
		char sign = 1;
		for (int i = 0; i < acc; i++) {
			double m = c * (c+1) * (c+2);
			pi = pi + sign * 4 / m;
			c = c + 2;
			sign = -sign;
			printf("pi %lf m %lf\n", pi, m);
		}
		printf("with acc %d, pi ~= %1.10f\n", acc, pi);
	}

	return 0;

	String vec = vec_new(5);
	string_concat(&vec, "1");

	while(1) {
		printf("%s\n", string_get_str(&vec));

		int buffer;
		scanf("%u", &buffer);
		if (buffer == 0) {
			break;
		}
		fibbonacci_push(&vec, buffer);
	}

	return 0;
}
