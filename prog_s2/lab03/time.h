#include "stddef.h"
#include "stdint.h"

typedef struct {
	uint16_t hours;
	uint8_t minutes;
} Time;


// Создаёт новую структуру для хранения результата
Time* time_from_hm_string(char* string);

Time* time_from_hm(uint16_t hours, uint8_t minutes);

Time* time_from_m(uint32_t minutes);

void time_destroy(Time* time);

uint32_t time_calc_minutes(const Time* time);

// Создаёт новую структуру для хранения результата
Time* time_elapsed(const Time* earlier, const Time* later);

void time_print(const Time* time);

// Создаёт новую структуру для хранения результата
Time* time_total(Time** array, size_t array_length);

// Возвращает указатель на минимальный элемент
Time* time_smallest(Time** array, size_t array_length);