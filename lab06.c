#include "stdio.h"

unsigned char months_table[2][12] = {
	{ // Не високосный год
		31, // Январь
		28, // Февраль
		31, // Март
		30, // Апрель
		31, // Май
		30, // Июнь
		31, // Июль
		31, // Август
		30, // Сентябрь
		31, // Октябрь
		30, // Ноябрь
		31  // Декабрь
	},
	{ // Високосный год
		31, // Январь
		29, // Февраль
		31, // Март
		30, // Апрель
		31, // Май
		30, // Июнь
		31, // Июль
		31, // Август
		30, // Сентябрь
		31, // Октябрь
		30, // Ноябрь
		31  // Декабрь
	}
};

unsigned char is_leap_year(unsigned int year) {
	if (year % 400 == 0) {
		return 1;
	} else if (year % 100 == 0) {
		return 0;
	} else if (year % 4 == 0) {
		return 1;
	} else {
		return 0;
	}
}

unsigned int calculate_days() {
	unsigned int year;
	unsigned char month;
	unsigned char day;
	scanf("%d %hhd %hhd", &year, &month, &day);

	unsigned int epoch = 0;

	for(unsigned int i = 1; i <= year; i++) {
		if (is_leap_year(i)) {
			epoch += 366;
		} else {
			epoch += 365;
		}
	}

	unsigned char leap_year = is_leap_year(year);

	for(unsigned int i = 0; i < month - 1; i++) {
		epoch += months_table[leap_year][i];
	}

	epoch += day - 1;

	return epoch;
}

int main(int argc, char** argv) {
	while(1) {
		unsigned int days1 = calculate_days();
		unsigned int days2 = calculate_days();

		unsigned int differance;
		if (days2 > days1) {
			differance = days2 - days1;
		} else {
			differance = days1 - days2;
		}

		printf("There are %d days between two dates.\n", differance);
	}

	return 0;
}
