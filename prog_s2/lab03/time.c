#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

// Создаёт новую структуру для хранения результата
Time* time_from_hm_string(char* string) {
	char *ptr = strchr(string, ':');
	if(!ptr) {
		return NULL;
	}
	*ptr = '\0';
	Time* ret = (Time*) malloc(sizeof(Time));
	ret->hours = atoi(string);
	ret->minutes = atoi(ptr+1);
	*ptr = ':';
	return ret;
}

Time* time_from_hm(uint16_t hours, uint8_t minutes) {
	Time* ret = (Time*) malloc(sizeof(Time));
	ret->hours = hours;
	ret->minutes = minutes;
	return ret;
}

Time* time_from_m(uint32_t minutes) {
	Time* ret = (Time*) malloc(sizeof(Time));
	ret->hours = minutes / 60;
	ret->minutes = minutes % 60;
	return ret;
}

void time_destroy(Time* time) {
	free(time);
}

uint32_t time_calc_minutes(const Time* time) {
	return (uint32_t) time->hours * 60 + (uint32_t) time->minutes;
}

// Создаёт новую структуру для хранения результата
Time* time_elapsed(const Time* earlier, const Time* later) {
	uint32_t minutes1 = time_calc_minutes(earlier);
	uint32_t minutes2 = time_calc_minutes(later);

	uint32_t minutes_elapsed = minutes2 - minutes1;

	return time_from_m(minutes_elapsed);
}

void time_print(const Time* time) {
	printf("%02d:%02d", time->hours, time->minutes);
}

// Создаёт новую структуру для хранения результата
Time* time_total(Time** array, size_t array_length) {
	uint32_t minutes_elapsed = 0;
	for(size_t i = 0; i != array_length; i++) {
		minutes_elapsed += time_calc_minutes(array[i]);
	}
	return time_from_m(minutes_elapsed);
}

// Возвращает указатель на минимальный элемент
Time* time_smallest(Time** array, size_t array_length) {
	if (array_length == 0) {
		return NULL;
	}

	Time* min_time = array[0];
	uint32_t min_minutes = time_calc_minutes(min_time);
	for(size_t i = 1; i != array_length; i++) {
		if (time_calc_minutes(array[i]) < min_minutes) {
			min_time = array[i];
		}
	}
	return min_time;
}