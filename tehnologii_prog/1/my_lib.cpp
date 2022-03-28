#include "my_lib.h"
#include <cstdlib>
#include <cstdio>

void generate_matrix(char mat[25][25], int m_size) {
	srand(1337);

	for(char y = 0; y < m_size; y++) {
		for(char x = 0; x < m_size; x++) {
			mat[y][x] = rand() % 7 - 3;
		}
	}
}

void print_matrix(char mat[25][25], int m_size) {
	for(char y = 0; y < m_size; y++) {
		for(char x = 0; x < m_size; x++) {
			printf("%3d", mat[y][x]);
		}
		printf("\n");
	}
	printf("\n");
}

int sum_main_diagonal(char mat[25][25], int m_size) {
	int sum = 0;
	for(int i = 0; i < m_size; i++) {
		sum += mat[i][i];
	}
	return sum;
}

void elements_outside_of_threshold(char mat[25][25], int m_size, char threshold, int* above, int* below) {
	*below = 0;
	*above = 0;
	for(char y = 0; y < m_size; y++) {
		for(char x = 0; x < m_size; x++) {
			if (mat[y][x] < threshold) {
				(*below)++;
			} else if (mat[y][x] > threshold) {
				(*above)++;
			}
		}
	}
}

void locate_biggest_element(char mat[25][25], int m_size, char* y, char* x) {
	*y = 0;
	*x = 0;
	char max = mat[0][0];
	for(char iy = 0; iy < m_size; iy++) {
		for(char ix = 0; ix < m_size; ix++) {
			if(mat[iy][ix] > max) {
				max = mat[iy][ix];
				*y = iy;
				*x = ix;
			}
		}
	}
}

void remove_cross(char mat[25][25], int* m_size, char x, char y) {
	for(char iy = 0; iy < *m_size; iy++) {
		for(char ix = x; ix < *m_size-1; ix++) {
			mat[iy][ix] = mat[iy][ix+1];
		}
	}
	for(char ix = 0; ix < *m_size; ix++) {
		for(char iy = y; iy < *m_size-1; iy++) {
			mat[iy][ix] = mat[iy+1][ix];
		}
	}
	*m_size--;
}

char count_rows_with_zeroes(char mat[25][25], int m_size) {
	char rows_with_zeroes = 0;
	for(char y = 0; y < m_size; y++) {
		bool has_zero = false;
		for(char x = 0; x < m_size; x++) {
			if(mat[y][x] == 0) {
				has_zero = true;
			}
		}
		if(has_zero) {
			rows_with_zeroes++;
		}
	}
	return rows_with_zeroes;
}