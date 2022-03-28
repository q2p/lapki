#include <cstdio>
#include <cstdlib>
#include <clocale>
#include "my_lib.h"

// Вариант 11
int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Russian");

	int m_size;

	char mat[25][25];

	scanf("%d", &m_size);

	generate_matrix(mat, m_size);

	printf("Исходная матрица:\n");
	print_matrix(mat, m_size);

	int sum = sum_main_diagonal(mat, m_size);
	printf("Сумма элементов на главной диагонали: %d\n\n", sum);

	int above;
	int below;
	elements_outside_of_threshold(mat, m_size, sum, &above, &below);
	
	printf("Количество элементов меньше суммы: %d\n", below);
	printf("Количество элементов большу суммы: %d\n", above);
	
	return 0;
}