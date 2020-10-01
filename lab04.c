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

typedef struct {
	unsigned int  year;
	unsigned char month;
	unsigned char day;
} Date;

Date read_date(char *is_error) {
	Date ret;
	scanf("%d %hhd %hhd", &ret.year, &ret.month, &ret.day);

	if (
		ret.year  < 1 && ret.year  > 10000                                      && // Если год   в пределе [1..10000]
		ret.month < 1 && ret.month > 12                                         && // Если месяц в пределе [1..12]
		ret.day   < 1 && ret.day   > months_table[is_leap_year(year)][ret.month-1] // Если день  в пределе [1..(Количество дней в данном месяце)]
	) {
		printf("Date is malformed.")
		*is_error = 1;
	}

	return ret;
}

unsigned int calculate_days(Date *date) {
	unsigned int absolute_days = 0;

	for(unsigned int i = 1; i < date->year; i++) {
		if (is_leap_year(i)) {
			absolute_days += 366;
		} else {
			absolute_days += 365;
		}
	}

	unsigned char leap_year = is_leap_year(date->year);

	for(unsigned int i = 0; i < date->month - 1; i++) {
		absolute_days += months_table[leap_year][i];
	}

	absolute_days += date->day - 1;

	return absolute_days;
}


unsigned int calculate_days2(Date *date) {
	unsigned int absolute_days = 0;

	unsigned int i = date->year;
	for(; i > 400; i -= 400) {
		absolute_days += 366 *  97;
		absolute_days += 365 * 303;
	}
	for(; i > 100; i -= 100) {
		absolute_days += 366 * 24;
		absolute_days += 365 * 76;
	}
	for(; i > 4; i -= 4) {
		absolute_days += 366 * 1;
		absolute_days += 365 * 3;
	}
	for(; i > 1; i -= 1) {
		absolute_days += 365;
	}

	unsigned char leap_year = is_leap_year(date->year);

	for(unsigned int i = 0; i < date->month - 1; i++) {
		absolute_days += months_table[leap_year][i];
	}

	absolute_days += date->day - 1;

	return absolute_days;
}

int main(int argc, char** argv) {
	char is_error = 0;
	while(1) {
		Date date1 = read_date(&is_error);
		if (is_error)
			return 1;
		Date date2 = read_date(&is_error);
		if (is_error)
			return 1;

		unsigned int days1 = calculate_days(&date1);
		unsigned int days2 = calculate_days2(&date2);

		unsigned int differance;
		if (days2 > days1) {
			differance = days2 - days1;
		} else {
			differance = days1 - days2;
		}

		printf("%d\n%d\n", days1, days2);

		printf("There are %d days between two dates.\n", differance);
	}

	return 0;
}
