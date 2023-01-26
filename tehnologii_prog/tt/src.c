// Вариант 11

#include "stdafx.h"
#include <cstdio>
#include <cstdlib>
#include <clocale>

// Заполняет матрицу случайными числами от -3 до +3
void generate_matrix(char mat[25][25], char m_size) {
	srand(228);

	for(int y = 0; y < m_size; y++) {
		for(int x = 0; x < m_size; x++) {
			mat[y][x] = rand() % 7 - 3;
		}
	}
}

// Выводит матрицу
void print_matrix(char mat[25][25], char m_size) {
	for(int y = 0; y < m_size; y++) {
		for(int x = 0; x < m_size; x++) {
			printf("%3d", mat[y][x]);
		}
		printf("\n");
	}
	printf("\n");
}

// Вычисляет сумму элементов главной диагонали
int sum_main_diagonal(char mat[25][25], char m_size) {
	int sum = 0;
	for(int i = 0; i < m_size; i++) {
		sum += mat[i][i];
	}
	return sum;
}

// Вычисляет количество элементов, которые больше или меньше указаного порога
void elements_outside_of_threshold(char mat[25][25], char m_size, char threshold, int* above, int* below) {
	*below = 0;
	*above = 0;
	for(int y = 0; y < m_size; y++) {
		for(int x = 0; x < m_size; x++) {
			if(mat[y][x] < threshold) {
				(*below)++;
			} else if(mat[y][x] > threshold) {
				(*above)++;
			}
		}
	}
}

// Вариант 11
int _tmain(int argc, _TCHAR* argv[]) {
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

	printf("Количество элементов меньше суммы: %d\n\n", below);
	printf("Количество элементов больше суммы: %d\n\n", above);
	
	return 0;
}
