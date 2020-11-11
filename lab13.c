#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

typedef struct __STRING {
	void* ptr;
	size_t len;
	size_t capacity;
} String;

String vec_new(size_t starting_capacity) {
	String s;
	if(starting_capacity != 0) {
		s.ptr = malloc(starting_capacity);
	} else {
		s.ptr = NULL;
	}
	s.len = 0;
	s.capacity = starting_capacity;
	return s;
}

String str_new(size_t starting_capacity) {
	if (starting_capacity == 0) {
		starting_capacity = 1;
	}
	String s = vec_new(starting_capacity);
	s.len = 1;
	*((char*)s.ptr) = '\0';
	return s;
}

void string_free(String* vec) {
	if(vec->ptr) {
		free(vec->ptr);
	}
}

void vec_reserve(String* vec, size_t additional) {
	size_t req = vec->len+additional;
	if (req > vec->capacity) {
		vec->capacity = req + req / 2;
		if(vec->ptr) {
			vec->ptr = realloc(vec->ptr, vec->capacity);
		} else {
			vec->ptr = malloc(vec->capacity);
		}
	}
}

void vec_copy(String* to, String* from) {
	to->len = 0;
	vec_reserve(to, from->len);
	to->len = from->len;
	memcpy(to->ptr, from->ptr, from->len);
}

void vec_push(String* vec, const void* value, size_t len) {
	vec_reserve(vec, len);
	memcpy(vec->ptr+vec->len, value, len);
	vec->len += len;
}

void string_push(String* vec, char value) {
	vec_reserve(vec, 1);
	((char*)vec->ptr)[vec->len-1] = value;
	((char*)vec->ptr)[vec->len  ] = '\0';
	vec->len++;
}

void string_concat(String* vec, char* append) {
	size_t sl = strlen(append);
	vec_reserve(vec, sl);
	memcpy(vec->ptr + vec->len - 1, append, sl + 1);
	vec->len = vec->len + sl;
}

char* string_idx(String* vec, size_t idx) {
	if (idx >= vec->len - 1) {
		return NULL;
	} else {
		return vec->ptr+idx;
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

typedef uint32_t BT;
typedef uint64_t OT;
#define BASE 0x10000000
const uint8_t zero = 0;

void make_same_size(String* a, String* b) {
	if (b->len < a->len) {
		String* t = a;
		a = b;
		b = t;
	}
	while(a->len != b->len) {
		vec_push(a, &zero, 1);
	}
}

void big_trim(String* big) {
	while(big->len > sizeof(BT) && *(BT*)(big->ptr+big->len-sizeof(BT)) == 0) {
		big->len -= sizeof(BT);
	}
}

String big_set(String* big, uint64_t value) {
	big->len = 0;
	do {
		BT place = value % BASE;
		value /= BASE;
		vec_push(big, &place, sizeof(BT));
	} while(value != 0);
}

String big_new(uint64_t value) {
	String big = vec_new(1);

	big_set(&big, value);

	return big;
}

String big_to_string(String* big) {
	String ret = str_new(1);
	
	for(int i = 0; i != big->len; i += sizeof(BT)) {
		BT v = *(BT*)(big->ptr + i);
		for(int j = 0; j != 2; j++) {
			string_push(&ret, '0' + v % 10);
			v /= 10;
		}
	}

	char* a = ret.ptr;
	char* b = ret.ptr + ret.len - 2;
	while (a < b) {
		*a ^= *b;
		*b ^= *a;
		*a ^= *b;
		a++;
		b--;
	}

	return ret;
}

uint8_t big_greater(String* a, String* b) {
	make_same_size(a, b);

	for(size_t i = a->len; i != 0; i -= sizeof(BT)) {
		size_t off = i - sizeof(BT);
		BT av = *(BT*)(a->ptr + off);
		BT bv = *(BT*)(b->ptr + off);
		if (av > bv) {
			return 1;
		}
		if (av < bv) {
			return 0;
		}
	}
	
	return 0;
}

void big_print(String* big) {
	String tp = big_to_string(big);
	
	printf("%s\n", (char*) tp.ptr);

	string_free(&tp);
}

void big_add(String* to, String* operand) {
	make_same_size(to, operand);

	BT carry = 0;
	for(size_t i = 0; i != to->len; i += sizeof(BT)) {
		BT sum = *(BT*)(to->ptr + i) + *(BT*)(operand->ptr + i) + carry;
		*(BT*)(to->ptr + i) = sum % BASE;
		carry = sum / BASE;
	}
	if (carry) {
		string_push(to, 1);
	}
}

void big_sub(String* to, String* operand) {
	make_same_size(to, operand);

	BT carry = 0;
	for(size_t i = 0; i != to->len; i += sizeof(BT)) {
		BT t = (BASE + *(BT*)(to->ptr + i)) - (carry + *(BT*)(operand->ptr + i));
		*(BT*)(to->ptr + i) = t % BASE;
		carry = (t >= BASE) ? 0 : 1;
	}

	big_trim(to);
}

void big_mul(String* to, String* a, String* b) {
	to->len = 0;
	size_t nl = (a->len + b->len) * sizeof(BT);
	
	vec_reserve(to, nl);
	to->len = nl;
	memset(to->ptr, 0, nl);

	for(size_t i = 0; i != a->len; i += sizeof(BT)) {
		BT carry = 0;
		for(size_t j = 0; j != b->len; j += sizeof(BT)) {
			BT av = *(BT*)(a->ptr + i);
			BT bv = *(BT*)(b->ptr + j);
			BT* tv = (BT*)(to->ptr + i + j);
			OT prod = (OT)*tv + ((OT)av * (OT)bv) + (OT)carry;
			*tv = prod % BASE;
			carry = prod / BASE;
		}
		if (carry) {
			*(BT*)(to->ptr + i + b->len) += carry;
		}
	}

	big_trim(to);
}

void big_mul_one_place(String* a, BT b) {
	BT carry = 0;
	for(size_t i = 0; i != a->len; i += sizeof(BT)) {
		BT* av = (BT*)(a->ptr + i);
		OT prod = *av * b + carry;
		*av = prod % BASE;
		carry = prod / BASE;
	}
	if (carry) {
		vec_push(a, &carry, sizeof(BT));
	}
}

void print_pi_up_to2(uint8_t prescision) {
	// C = 426880 * sqrt(10005) ~= 42698670.66633339
	const double C = 42698670.66633339;

	uint64_t L = 13591409;
	const uint64_t L_inc = 545140134;

	double X = 1;
	const double X_mul = -262537412640768000;

	uint32_t M_c = 1;
	uint32_t M_z = 1;

	double pi_c = 1;
	double pi_z = 1;
	for (uint8_t q = 0; q != prescision; q++) {
		uint32_t q13 = q+1;
		q13 = q13 * q13 * q13;
		uint32_t _12q = 12 * q;

		double nz = X * M_z;

		pi_c = pi_c * nz + (M_c * L) * pi_z;
		pi_z = pi_z * nz;

		L += L_inc;
		X *= X_mul;
		M_c *= ((_12q + 2)*(_12q + 6)*(_12q + 10));
		M_z *= q13;
	}

	double pi = C * (pi_z / pi_c);

	printf("with acc %d, pi ~= %1.60lf\n", prescision, pi);
}

void print_pi_up_to(uint16_t prescision) {
	String c = big_new(0);
	String z = big_new(1);

	uint64_t prev = 2;
	uint8_t sign = 1;

	String t = vec_new(1);
	String nz_big = vec_new(1);

	for (uint16_t q = 0; q != prescision; q++) {
		uint64_t nz = ((prev++) >> 1) * prev++ * (prev >> 1);
		big_set(&nz_big, nz);

		big_mul(&t, &c, &nz_big);

		if (sign) {
			big_add(&t, &z);
		} else {
			big_sub(&t, &z);
		}
		sign = !sign;
		big_mul(&c, &z, &nz_big);

		vec_copy(&z, &c);
		vec_copy(&c, &t);
	}

	string_free(&nz_big);
	string_free(&t);

	printf("with acc %d, pi ~= 3.", prescision);
	print_fraction(&c, &z, 16);
	printf("\n");

	string_free(&z);
	string_free(&c);
}

void print_fraction(String* c, String* z, uint8_t digits) {
	while(digits--) {
		big_mul_one_place(c, 10);
		uint8_t times = 0;
		while(big_greater(c, z)) {
			big_sub(c, z);
			times++;
		}
		printf("%d", times);
	}
}

int main(int argc, char** argv) {
	while (1) {
		int acc;
		scanf("%i", &acc);
		print_pi_up_to((uint16_t) acc);
	}

	return 0;

	String a = big_new(129);
	String b = big_new(13370322);
	String to = big_new(13370322);

	big_print(&a);
	big_print(&b);

	big_mul(&to, &a, &b);

	big_print(&to);

	string_free(&to);
	string_free(&b);
	string_free(&a);

	return 0;

	String vec = vec_new(5);
	vec_push(&vec, "1", 2);
	fibbonacci_push(&vec, 3);

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
