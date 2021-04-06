#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// ==== SHARED TYPE ====

typedef int32_t Data;

static inline void data_swap(Data* d1, Data* d2) {
	*d1 ^= *d2;
	*d2 ^= *d1;
	*d1 ^= *d2;
}

// ==== HEAP (PRIORITY QUEUE) ====

// not 0 if `d1` should be the root
static inline uint8_t heap_compare(Data* d1, Data* d2) {
	return *d1 > *d2;
}

typedef struct {
	Data* data;
	size_t len;
	size_t capacity;
} Heap;

static inline size_t heap_parent(size_t i) {
	return (i - 1) >> 1;
}

static inline size_t heap_left(size_t i) {
	return (i << 1) + 1;
}

static inline size_t heap_right(size_t i) {
	return (i << 1) + 2;
}

static inline uint8_t heap_is_empty(Heap* heap) {
	return heap->len == 0;
}

static inline uint8_t heap_can_put(Heap* heap) {
	return heap->len != heap->capacity;
}

/// Returns `NULL` if can't allocate
static inline void* heap_init(Heap* heap, size_t capacity) {
	assert(capacity > 0);
	heap->len = 0;
	heap->capacity = capacity;
	heap->data = malloc(capacity * sizeof(Data));
	return heap->data;
}

static inline void heap_free(Heap* heap) {
	free(heap->data);
}

static inline void heap_clear(Heap* heap) {
	heap->len = 0;
}

static inline size_t heap_len(Heap* heap) {
	return heap->len;
}

static inline Data* heap_peek(Heap* heap) {
	assert(!heap_is_empty(heap));
	return heap->data;
}

void heap_put(Heap* heap, Data* new) {
	assert(heap_can_put(heap));

	size_t i = heap->len;
	heap->len++;
	heap->data[i] = *new;
	while(i != 0) {
		size_t hp = heap_parent(i);
		Data* d1 = &heap->data[i];
		Data* d2 = &heap->data[hp];
		if(heap_compare(d2, d1)) {
			break;
		}
		data_swap(d1, d2);
		i = hp;
	}
}

Data heap_pop(Heap* heap) {
	assert(!heap_is_empty(heap));

	Data ret = *heap->data;

	heap->len--;
	*heap->data = heap->data[heap->len];
	size_t i = 0;
	while(1) {
		size_t hl = heap_left(i);
		if(hl >= heap->len) {
			break;
		}
		size_t hr = heap_right(i);
		Data* dl = &heap->data[hl];
		Data* dr = &heap->data[hr];
		if(heap_compare(dr, dl)) {
			hl = hr;
			dl = dr;
		}
		Data* di = &heap->data[i];
		if(heap_compare(di, dl)) {
			break;
		}
		data_swap(di, dl);
		i = hl;
	}

	return ret;
}

// ==== CYCLICAL ARRAY DEQUE ====

typedef struct {
	Data* data;
	size_t start;
	size_t end;
	size_t capacity;
	uint8_t is_empty;
} Deque;

static inline uint8_t deque_is_empty(Deque* deque) {
	assert(!deque->is_empty || deque->start == deque->end);
	return deque->is_empty;
}

static inline uint8_t deque_can_put(Deque* deque) {
	assert(!deque->is_empty || deque->start == deque->end);
	return deque->is_empty || deque->start != deque->end;
}

/// Returns `NULL` if can't allocate
static inline void* deque_init(Deque* deque, size_t capacity) {
	assert(capacity > 0);
	deque->start = 0;
	deque->end = 0;
	deque->capacity = capacity;
	deque->is_empty = 1;
	deque->data = malloc(capacity * sizeof(Data));
	return deque->data;
}

static inline void deque_free(Deque* deque) {
	free(deque->data);
}

static inline void deque_clear(Deque* deque) {
	deque->start = 0;
	deque->end = 0;
	deque->is_empty = 1;
}

void deque_put_head(Deque* deque, Data* new) {
	assert(deque_can_put(deque));
	deque->is_empty = 0;
	deque->start = (deque->start + deque->capacity - 1) % deque->capacity;
	deque->data[deque->start] = *new;
}

void deque_put_tail(Deque* deque, Data* new) {
	assert(deque_can_put(deque));
	deque->is_empty = 0;
	deque->data[deque->end] = *new;
	deque->end = (deque->end + 1) % deque->capacity;
}

static inline Data* deque_peek_head(Deque* deque) {
	assert(!deque_is_empty(deque));
	return &deque->data[deque->start];
}

static inline Data* deque_peek_tail(Deque* deque) {
	assert(!deque_is_empty(deque));
	return &deque->data[(deque->end + deque->capacity - 1) % deque->capacity];
}

static inline Data deque_pop_head(Deque* deque) {
	assert(!deque_is_empty(deque));
	Data ret = deque->data[deque->start];
	deque->start = (deque->start + 1) % deque->capacity;
	deque->is_empty = deque->start == deque->end;
	return ret;
}

static inline Data deque_pop_tail(Deque* deque) {
	assert(!deque_is_empty(deque));
	deque->end = (deque->end + deque->capacity - 1) % deque->capacity;
	deque->is_empty = deque->start == deque->end;
	return deque->data[deque->end];
}

static inline Data* deque_at_head(Deque* deque, size_t i) {
	assert(!deque_is_empty(deque));
	return &deque->data[(deque->start + i) % deque->capacity];
}

static inline Data* deque_at_tail(Deque* deque, size_t i) {
	assert(!deque_is_empty(deque));
	return &deque->data[(deque->end + deque->capacity - i - 1) % deque->capacity];
}

// ==== MANIPULATION ====

void test_heap_put(Heap* heap, Deque* deque) {
	Data data;
	scanf("%d", &data);
	if(heap_can_put(heap)) {
		heap_put(heap, &data);
	} else {
		puts("Heap is full");
	}
}
void test_heap_clear(Heap* heap, Deque* deque) {
	heap_clear(heap);
}
void test_heap_peek(Heap* heap, Deque* deque) {
	if(heap_is_empty(heap)) {
		puts("Heap is empty");
	} else {
		printf("%d\n", *heap_peek(heap));
	}
}
void test_heap_pop(Heap* heap, Deque* deque) {
	if(heap_is_empty(heap)) {
		puts("Heap is empty");
	} else {
		printf("%d\n", heap_pop(heap));
	}
}
void test_heap_len(Heap* heap, Deque* deque) {
	printf("%ld\n", heap_len(heap));
}
void test_heap_is_empty(Heap* heap, Deque* deque) {
	puts(heap_is_empty(heap) ? "Heap is empty" : "Heap is not empty");
}
void test_heap_can_put(Heap* heap, Deque* deque) {
	puts(heap_can_put(heap) ? "Heap is not full" : "Heap is full");
}
void test_heap_print(Heap* heap, Deque* deque) {
	printf("{ ");
	for(size_t i = 0; i != heap->len; i++) {
		printf("%d ", heap->data[i]);
	}
	printf("}\n");
}
void test_heap_dump(Heap* heap, Deque* deque) {
	printf("{ ");
	while(!heap_is_empty(heap)) {
		printf("%d ", heap_pop(heap));
	}
	printf("}\n");
}
void test_deque_put_head(Heap* heap, Deque* deque) {
	Data data;
	scanf("%d", &data);
	if(deque_can_put(deque)) {
		deque_put_head(deque, &data);
	} else {
		puts("Deque is full");
	}
}
void test_deque_put_tail(Heap* heap, Deque* deque) {
	Data data;
	scanf("%d", &data);
	if(deque_can_put(deque)) {
		deque_put_tail(deque, &data);
	} else {
		puts("Deque is full");
	}
}
void test_deque_at_head(Heap* heap, Deque* deque) {
	size_t i;
	scanf("%lu", &i);
	if(deque_is_empty(deque)) {
		puts("Deque is empty");
	} else {
		deque_at_head(deque, i);
	}
}
void test_deque_at_tail(Heap* heap, Deque* deque) {
	size_t i;
	scanf("%lu", &i);
	if(deque_is_empty(deque)) {
		puts("Deque is empty");
	} else {
		deque_at_tail(deque, i);
	}
}
void test_deque_clear(Heap* heap, Deque* deque) {
	deque_clear(deque);
}
void test_deque_peek_head(Heap* heap, Deque* deque) {
	if(deque_is_empty(deque)) {
		puts("Deque is empty");
	} else {
		printf("%d\n", *deque_peek_head(deque));
	}
}
void test_deque_peek_tail(Heap* heap, Deque* deque) {
	if(deque_is_empty(deque)) {
		puts("Deque is empty");
	} else {
		printf("%d\n", *deque_peek_tail(deque));
	}
}
void test_deque_pop_head(Heap* heap, Deque* deque) {
	if(deque_is_empty(deque)) {
		puts("Deque is empty");
	} else {
		printf("%d\n", deque_pop_head(deque));
	}
}
void test_deque_pop_tail(Heap* heap, Deque* deque) {
	if(deque_is_empty(deque)) {
		puts("Deque is empty");
	} else {
		printf("%d\n", deque_pop_tail(deque));
	}
}
void test_deque_is_empty(Heap* heap, Deque* deque) {
	puts(deque_is_empty(deque) ? "Deque is empty" : "Deque is not empty");
}
void test_deque_can_put(Heap* heap, Deque* deque) {
	puts(deque_can_put(deque) ? "Deque is not full" : "Deque is full");
}
void test_deque_print(Heap* heap, Deque* deque) {
	printf("\"");
	if(!deque_is_empty(deque)) {
		size_t i = deque->start;
		do {
			printf("%d ", deque->data[i]);
			i = (i + 1) % deque->capacity;
		} while(i != deque->end);
	}
	printf("\"\n");
}

typedef struct {
	char* command;
	void (*func)(Heap*, Deque*);
} ActionMap;

const ActionMap actions[] = {
	{ .command = "h_put",       .func = test_heap_put        },
	{ .command = "h_clear",     .func = test_heap_clear      },
	{ .command = "h_peek",      .func = test_heap_peek       },
	{ .command = "h_pop",       .func = test_heap_pop        },
	{ .command = "h_len",       .func = test_heap_len        },
	{ .command = "h_is_empty",  .func = test_heap_is_empty   },
	{ .command = "h_can_put",   .func = test_heap_can_put    },
	{ .command = "h_print",     .func = test_heap_print      },
	{ .command = "h_dump",      .func = test_heap_dump       },
	{ .command = "d_put_head",  .func = test_deque_put_head  },
	{ .command = "d_put_tail",  .func = test_deque_put_tail  },
	{ .command = "d_at_head",   .func = test_deque_at_head   },
	{ .command = "d_at_tail",   .func = test_deque_at_tail   },
	{ .command = "d_clear",     .func = test_deque_clear     },
	{ .command = "d_peek_head", .func = test_deque_peek_head },
	{ .command = "d_peek_tail", .func = test_deque_peek_tail },
	{ .command = "d_pop_head",  .func = test_deque_pop_head  },
	{ .command = "d_pop_tail",  .func = test_deque_pop_tail  },
	{ .command = "d_is_empty",  .func = test_deque_is_empty  },
	{ .command = "d_can_put",   .func = test_deque_can_put   },
	{ .command = "d_print",     .func = test_deque_print     },
};

enum {
	 HEAP_CAPACITY = 8,
	DEQUE_CAPACITY = 8,
};


// ==== MAIN FUNCTION ====

int main(int argc, char** argv) {
	Heap heap;
	Deque deque;

	if(!heap_init(&heap, HEAP_CAPACITY)) {
		return 1;
	}
	if(!deque_init(&deque, DEQUE_CAPACITY)) {
		heap_free(&heap);
		return 1;
	}

	loop: while(1) {
		char input[80];
		scanf("%s", input);
		for(size_t i = 0; i != sizeof(actions) / sizeof(ActionMap); i++) {
			if(strcmp(input, actions[i].command) == 0) {
				actions[i].func(&heap, &deque);
				goto loop;
			}
		}
		if(strcmp(input, "exit") == 0) {
			break;
		}
		puts("Unknown command, try using:");
		for(size_t i = 0; i != sizeof(actions) / sizeof(ActionMap); i++) {
			puts(actions[i].command);
		}
	}
	
	heap_free(&heap);
	deque_free(&deque);

	return 0;
}
