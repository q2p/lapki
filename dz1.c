#include "stdio.h"
#include "math.h"

// Оставляет числу лишь две цифры после запятой
double prec(double value) {
	// Приводим к int64, чтобы избавится от погрешностей double и отрицательных нулей
	return (double)((long long)round(value * 100.0)) / 100.0;
}

int main(int argc, char** argv) {
	double a, b, c;

	// Читаем ввод
	printf("Please enter an `a` value: ");
	scanf("%lf", &a);
	printf("Please enter a  `b` value: ");
	scanf("%lf", &b);
	printf("Please enter a  `c` value: ");
	scanf("%lf", &c);
	
	// Выводим получившееся уравнение
	printf("Quadratic equation %g * x^2 + %g * x + %g ", a, b, c);

	// Проверка деления на ноль
	if (a == 0.0) {
		double x = prec(-c / b);
		printf("has one root:\nx = %g.\n", x);
	} else {
		double discriminant = b*b - 4 * a * c;

		if (discriminant == 0) {
			double x = prec(-b / (2 * a));
			printf("has one root:\nx = %g.\n", x);
		} else if (discriminant > 0) {
			double sqrt_d = sqrt(discriminant);
			double x1 = prec((-b + sqrt_d) / (2 * a));
			double x2 = prec((-b - sqrt_d) / (2 * a));
			printf("has two roots:\nx1 = %g;\nx2 = %g.\n", x1, x2);
		} else {
			printf("has two complex roots:\n");
			// Дискриминант отрицательный, корни - комплексные
			double sqrt_d = sqrt(-discriminant);
			double x_imaginary = prec(sqrt_d / (2 * a)); // Мнимая часть
			if (b != 0) { // Если есть действительная часть
				double x_real = prec(-b / (2 * a)); // Действительная часть
				printf("x1 = %g + i*%g;\nx2 = %g - i*%g.\n", x_real, x_imaginary, x_real, x_imaginary);
			} else {
				printf("x1 =  i*%g;\nx2 = -i*%g.\n", x_imaginary, x_imaginary);
			}
		}
	}

	return 0;
}
