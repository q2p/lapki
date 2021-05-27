#include "stdio.h"
#include "math.h"
#include "string.h"
#include "stdint.h"
#include "stddef.h"
#include "stdlib.h"

void printArr(int32_t *array, size_t count){
	for(size_t i = 0; i < count; i++){
		printf("%d ", array[i]);
	}
	printf("\n");
}

void quickSort(int32_t *array, size_t first, size_t last) {
	if (first >= last) {
		return;
	}
	size_t left = first;
	size_t right = last;
	size_t middle = array[(left + right) / 2];
	do {
		while (array[left] < middle) {
			left++;
		}
		while (array[right] > middle) {
			right--;
		}
		if (left <= right) {
			int32_t t = array[left];
			array[left] = array[right];
			array[right] = t;
			left++;
			right--;
		}
	} while (left <= right);
	quickSort(array, first, right);
	quickSort(array, left, last);
}

int main(int argc, char** argv){
	const size_t count = 12;
	int32_t* array = malloc(sizeof(int32_t)*count);
	for(size_t i = 0; i != count; i++) {
		array[i] = 1 + rand() % 31;
	}
	printArr(array, count);
	quickSort(array, 0, count-1);
	printArr(array, count);
	free(array);
	return 0;
}