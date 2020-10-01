#include "stdio.h"

unsigned char months_table[2][12] = {
	{ // Не високосный год
		31, // Январь
		28, // Февраль [28]
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
		29, // Февраль [29]
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

	if (
		year  < 1 || year  > 10000                                  || // Если год   не в пределе [1..10000]
		month < 1 || month > 12                                     || // Если месяц не в пределе [1..12]
		day   < 1 || day   > months_table[is_leap_year(year)][month-1] // Если день  не в пределе [1..(Количество дней в данном месяце)]
	) {
		printf("Date is malformed.\n");
		return 0; // 0 - введётая дата содержит не допустимые значия
	}

	unsigned int absolute_days = 1; // Начинаем отсчёт с 1, чтобы не вернуть 0, при вводе "1г, 1м, 1д"

	for(unsigned int i = 1; i < year; i++) {
		if (is_leap_year(i)) { // Если год високосный
			absolute_days += 366;
		} else {
			absolute_days += 365;
		}
	}

	unsigned char *months = months_table[is_leap_year(year)]; // Таблица месяцев для текущего года
	for(unsigned int i = 0; i < month - 1; i++) {
		absolute_days += months[i];
	}

	absolute_days += day - 1;

	return absolute_days;
}

int main(int argc, char** argv) {
	unsigned int days1 = calculate_days();
	if (!days1) // Если в дату были введены недопустимые значения
		return 1;
	unsigned int days2 = calculate_days();
	if (!days2) // Если в дату были введены недопустимые значения
		return 1;

	unsigned int differance;
	if (days2 > days1) {
		differance = days2 - days1;
	} else {
		differance = days1 - days2;
	}

	printf("%d\n%d\n", days1, days2);
	if (differance == 0) {
		printf("The dates are the same.\n");
	} else {
		printf("There are %d days between the dates.\n", differance - 1);
	}

	return 0;
}
