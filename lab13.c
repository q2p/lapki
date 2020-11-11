#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

// === Работа с динамическими массивами ===

/// Динамический массив
typedef struct __VEC {
	/// Указатель на начало содержимого массива
	void* ptr;
	/// Количество байт, используемых массивом
	size_t len;
	/// Количество памяти, занимаемой массивом (вместимость)
	size_t capacity;
} Vec;

/// Создаёт динамический массив с заданной вместимостью
Vec vec_new(size_t starting_capacity) {
	Vec s;
	if(starting_capacity != 0) {
		s.ptr = malloc(starting_capacity);
	} else {
		s.ptr = NULL;
	}
	s.len = 0;
	s.capacity = starting_capacity;
	return s;
}

/// Освобождает динамический массив
void vec_free(Vec* vec) {
	if(vec->ptr) {
		free(vec->ptr);
	}
}

/// Резервирует `additional` байт для записи в динамический массив
void vec_reserve(Vec* vec, size_t additional) {
	size_t req = vec->len+additional;
	if (req > vec->capacity) {
		// Для экономии памяти мы увеличиваем вместимость массива в 1.5 раза,
		// что уменьшает количество аллокаций, но при этом не съедает память
		// так быстро, как множитель 2.0
		vec->capacity = req + req / 2;
		if(vec->ptr) {
			vec->ptr = realloc(vec->ptr, vec->capacity);
		} else {
			vec->ptr = malloc(vec->capacity);
		}
	}
}

/// Копирует значащие байты из `from` в `to`
void vec_copy(Vec* to, Vec* from) {
	to->len = 0;
	vec_reserve(to, from->len);
	to->len = from->len;
	memcpy(to->ptr, from->ptr, from->len);
}

/// Записывает `len` байт по указателю `value` в `vec`
void vec_push(Vec* vec, const void* value, size_t len) {
	vec_reserve(vec, len);
	memcpy(vec->ptr+vec->len, value, len);
	vec->len += len;
}

/// Отбрасывает незначащие байты и уменьшает объём занимаемый массивом
void vec_shrink(Vec* vec) {
	if (vec->ptr) {
		vec->capacity = vec->len;
		vec->ptr = realloc(vec->ptr, vec->capacity);
	}
}

// Функции для работы со строками, используя динамические массивы

/// Создаёт пустую строку с терминирующим нулём
Vec str_new(size_t starting_capacity) {
	if (starting_capacity == 0) {
		starting_capacity = 1;
	}
	Vec s = vec_new(starting_capacity);
	s.len = 1;
	*((char*)s.ptr) = '\0';
	return s;
}

/// Добавляет символ `value` в конец строки
void string_push(Vec* vec, char value) {
	vec_reserve(vec, 1);
	((char*)vec->ptr)[vec->len-1] = value;
	((char*)vec->ptr)[vec->len  ] = '\0';
	vec->len++;
}

/// Добавляет число `value` в конец строки
void string_push_int(Vec* vec, unsigned int value) {
	size_t old_len = vec->len;
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

// === Вычисление Фиббоначи ===

unsigned int fibbonacci_prev = 1;
unsigned int fibbonacci_curr = 1;

/// Записывает следующие N чисел Фиббоначи в строку
void fibbonacci_push(Vec* str, unsigned int how_many) {
	while (how_many) {
		string_push_int(str, fibbonacci_curr);

		unsigned int next = fibbonacci_prev + fibbonacci_curr;
		fibbonacci_prev = fibbonacci_curr;
		fibbonacci_curr = next;

		how_many--;
	}
}

// === Длинная арифметика ===

// Система счисления
#define BASE 0x10000000
/// Базовый тип (цифра)
typedef uint32_t BT;
/// Тип способный вместить `BASE * BASE`
typedef uint64_t OT;

/// Добавляет незначащие нули, чтобы переменные имели одинаковое количество цифр
void big_make_same_size(Vec* a, Vec* b) {
	if (b->len < a->len) {
		Vec* t = a;
		a = b;
		b = t;
	}
	
	size_t additional = b->len - a->len;
	vec_reserve(a, additional);
	memset(a->ptr + a->len, 0, additional);
	a->len = b->len;
}

/// Отбрасывает незначащие нули
void big_trim(Vec* big) {
	while(big->len > sizeof(BT) && *(BT*)(big->ptr+big->len-sizeof(BT)) == 0) {
		big->len -= sizeof(BT);
	}
}

/// Заводит в `big` значение `value`
Vec big_set(Vec* big, uint64_t value) {
	big->len = 0;
	do {
		BT place = value % BASE;
		value /= BASE;
		vec_push(big, &place, sizeof(BT));
	} while(value != 0);
}

/// Возвращает 1, если `a > b`, иначе 0
uint8_t big_greater(Vec* a, Vec* b) {
	big_make_same_size(a, b);

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

/// Складывает два числа, эквивалентно `to += operand`
void big_add(Vec* to, Vec* operand) {
	big_make_same_size(to, operand);

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

/// Вычитает два числа, эквивалентно `to -= operand`
void big_sub(Vec* to, Vec* operand) {
	big_make_same_size(to, operand);

	BT carry = 0;
	for(size_t i = 0; i != to->len; i += sizeof(BT)) {
		BT t = (BASE + *(BT*)(to->ptr + i)) - (carry + *(BT*)(operand->ptr + i));
		*(BT*)(to->ptr + i) = t % BASE;
		carry = (t >= BASE) ? 0 : 1;
	}

	big_trim(to);
}

/// Перемножает два числа, эквивалентно `to = a * b`
void big_mul(Vec* to, Vec* a, Vec* b) {
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

/// Умножает `a` на цифру `b`, эквивалентно `a *= b`
void big_mul_one_place(Vec* a, BT b) {
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

/// Выводит правильную дробь `c/z` с точностью до `digits` знаков после запятой
void big_print_fraction(Vec* c, Vec* z, uint8_t digits) {
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

// === Вычисление PI ===

/// Вычисляет PI, производя `prescision` итераций
///
/// Кол-во итераций   Точность
///               1   3.1
///              10   3.141
///             100   3.141592
///            1000   3.141592653
///           10000   3.141592653589
///          100000   3.1415926535897
///
void print_pi_up_to(unsigned int prescision) {
	Vec c = vec_new(1);
	Vec z = vec_new(1);
	Vec t = vec_new(1);
	Vec nz_big = vec_new(1);

	uint64_t prev = 2;
	uint8_t sign = 1;

	big_set(&c, 0);
	big_set(&z, 1);

	for (unsigned int q = prescision; q != 0; q--) {
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

	vec_free(&nz_big);
	vec_free(&t);

	printf("PI approximation with prescision of %u: PI ~= 3.", prescision);
	big_print_fraction(&c, &z, 15);
	printf("\n");

	vec_free(&z);
	vec_free(&c);
}

// === Начало программы ===

int main(int argc, char** argv) {
	// Строка хранящяя последовательность Фиббоначи
	Vec fibbonacci_sequence = vec_new(5);

	// Записываем первые 4 числа Фиббоначи
	vec_push(&fibbonacci_sequence, "1", 2);
	fibbonacci_push(&fibbonacci_sequence, 3);

	unsigned int input;

	while(1) {
		// Выводим последовательность Фиббоначи
		printf("Fibbonaci sequence: %s\n", fibbonacci_sequence.ptr);

		// Вставляем N последующих чисел Фиббоначи
		printf("How many additional Fibbonaci numbers do you want to append? (0 to skip)...\n");
		scanf("%u", &input);
		if (input == 0) {
			break;
		}
		fibbonacci_push(&fibbonacci_sequence, input);
	}

	// Освобождаем строку
	vec_free(&fibbonacci_sequence);

	while(1) {
		// Выводим приблизительное значение PI, после N итераций
		printf("How many iterations you want to do to approximate PI? (0 to exit)...\n");
		scanf("%u", &input);
		if (input == 0) {
			break;
		}
		print_pi_up_to(input);
	}

	return 0;
}
