// gcc -Wall -pedantic ./main.c && ./a.out

#include <stdint.h>
#include <malloc.h>
#include <stdio.h>

typedef struct Node {
	uint32_t value;
	struct Node* horizontal_next;
	struct Node* vertical_next;
} Node;

typedef struct {
	Node* root;
	Node* node;
} Builder;

Builder builder_new() {
	Builder builder = { .root = NULL, .node = NULL };
	return builder;
}

void builder_push(Builder* builder, uint32_t value) {
	Node* node = malloc(sizeof(Node));
	node->value = value;

	if (builder->root == NULL) {
		// Узлов нет
		builder->root = node;
		node->horizontal_next = NULL;
		node->vertical_next = NULL;
		builder->node = node;
		return;
	}

	if (builder->node->vertical_next == NULL) {
		// Список имеет один узел
		node->horizontal_next = NULL;
		node->vertical_next = builder->node;
		builder->node->vertical_next = node;
		return;
	}

	if (builder->node->horizontal_next == NULL) {
		// Длина списков равна
		node->horizontal_next = NULL;
		node->vertical_next = NULL;
		builder->node->horizontal_next = node;
		return;
	}

	// Длина списков различна
	node->vertical_next = builder->node->horizontal_next;
	node->horizontal_next = builder->node->vertical_next;
	builder->node->horizontal_next->vertical_next = node;
	builder->node = builder->node->horizontal_next;
}

Node* builder_finish(Builder* builder) {
	return builder->root;
}

typedef struct {
	Node* node;
	uint8_t read_current;
} Iterator;

Iterator iterator_new(Node* root) {
	Iterator iterator = { .node = root, .read_current = 0 };
	return iterator;
}

uint32_t* iterator_next(Iterator* iterator) {
	if (iterator->node == NULL) {
		return NULL;
	}

	uint32_t* ret;
	if(iterator->read_current) {
		ret = &iterator->node->vertical_next->value;
		iterator->node = iterator->node->horizontal_next;
	} else {
		ret = &iterator->node->value;
	}
	iterator->read_current = 1-iterator->read_current;
	return ret;
}

void free_list(Node* root) {
	while(root != NULL) {
		if(root->vertical_next) {
			free(root->vertical_next);
		}
		Node* to_free = root;
		root = root->horizontal_next;
		free(to_free);
	}
}

int main(int argc, char** argv) {
	Builder builder = builder_new();
	puts("Input values to be added to the list in a zig-zag sequence: ");
	while(1) {
		int value;
		scanf("%d", &value);
		if (value == 0) {
			break;
		}
		builder_push(&builder, value);
	}
	Node* root = builder_finish(&builder);
	
	Iterator iterator = iterator_new(root);
	puts("Values in the list printed in a zig-zag sequence:");
	while(1) {
		uint32_t* value = iterator_next(&iterator);
		if(value == NULL) {
			break;
		}
		printf("%d ", *value);
	}
	puts("");

	free_list(root);
	return 0;
}
