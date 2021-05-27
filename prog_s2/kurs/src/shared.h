#pragma once

#include <stddef.h>
#include "lib/shared.h"

const char* BIN_NAME;
void sort(void* array, size_t type_size, size_t count, Comparator comparator);