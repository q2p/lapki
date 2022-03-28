#ifndef MY_LIB_H
#define MY_LIB_H

void generate_matrix(char mat[25][25], int m_size);

void print_matrix(char mat[25][25], int m_size);

int sum_main_diagonal(char mat[25][25], int m_size);

void elements_outside_of_threshold(char mat[25][25], int m_size, char threshold, int* above, int* below);

void locate_biggest_element(char mat[25][25], char* y, char* x);

void remove_cross(char mat[25][25], int* m_size, char x, char y);

char count_rows_with_zeroes(char mat[25][25], int m_size);

#endif