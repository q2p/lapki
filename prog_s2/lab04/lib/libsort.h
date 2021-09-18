#pragma once

#include <stdint.h>
#include <stddef.h>

typedef int (*Comparator)(void*, void*);
typedef void (*SortFunc)(void*, size_t, size_t, Comparator);
void sort_binary_insertion(void* array, size_t count, size_t size, Comparator comparator);
void sort_shaker          (void* array, size_t count, size_t size, Comparator comparator);
void sort_quick_sort      (void* array, size_t count, size_t size, Comparator comparator);