#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct ListElement {
	uint32_t value;
	struct ListElement* to_tail;
	struct ListElement* to_head;
} ListElement;

typedef struct {
	ListElement* head;
	ListElement* tail;
} List;

void add_head(List* list, uint32_t value) {
	ListElement* ne = (ListElement*) malloc(sizeof(ListElement));
	ne->value = value;
	ne->to_tail = list->head;
	ne->to_head = NULL;
	list->head = ne;
	if(ne->to_tail == NULL) {
		list->tail = ne;
	} else {
		ne->to_tail->to_head = ne;
	}
}

void add_tail(List* list, uint32_t value) {
	ListElement* ne = (ListElement*) malloc(sizeof(ListElement));
	ne->value = value;
	ne->to_tail = NULL;
	ne->to_head = list->tail;
	list->tail = ne;
	if(ne->to_head == NULL) {
		list->head = ne;
	} else {
		ne->to_head->to_tail = ne;
	}
}

uint32_t* get_head(List* list) {
	if(list->head != NULL) {
		return &list->head->value;
	} else {
		return NULL;
	}
}

uint32_t* get_tail(List* list) {
	if(list->tail != NULL) {
		return &list->tail->value;
	} else {
		return NULL;
	}
}

uint8_t is_empty(List* list) {
	return list->head == NULL;
}

List make_list() {
	List ret = { .head = NULL, .tail = NULL };
	return ret;
}

void clear_list(List* list) {
	while(list->head != NULL) {
		ListElement* current = list->head;
		list->head = list->head->to_tail;
		free(current);
	}
	list->tail = NULL;
}

uint8_t pop_head(List* list, uint32_t* container) {
	if(list->head == NULL) {
		return 0;
	}
	ListElement* head = list->head;
	*container = head->value;
	list->head = head->to_tail;
	if(list->head == NULL) {
		list->tail = NULL;
	}
	free(head);
	return 1;
}

uint8_t pop_tail(List* list, uint32_t* container) {
	if(list->tail == NULL) {
		return 0;
	}
	ListElement* tail = list->tail;
	*container = tail->value;
	list->tail = tail->to_head;
	if(list->tail == NULL) {
		list->head = NULL;
	}
	free(tail);
	return 1;
}

void print_list(List* list) {
	printf("\"");
	ListElement* to_tail = list->head;
	while(to_tail != NULL) {
		printf("%d ", to_tail->value);
		to_tail = to_tail->to_tail;
	}
	printf("\"\n");
}

void deduplicate(List* list) {
	ListElement* from = list->head;
	while(from != NULL) {
		ListElement* next = from->to_tail;
		while(next != NULL) {
			ListElement* next_to_tail = next->to_tail;
			if(next->value == from->value) {
				next->to_head->to_tail = next_to_tail;
				if (next_to_tail == NULL) {
					list->tail = next->to_head;
				} else {
					next_to_tail->to_head = next->to_head;
				}
				free(next);
			}
			next = next_to_tail;
		}
		from = from->to_tail;
	}
}

uint32_t* get_at_head(List* list, size_t idx) {
	ListElement* to_tail = list->tail;
	while(1) {
		if(to_tail == NULL) {
			return NULL;
		}
		if(idx == 0) {
			return &to_tail->value;
		}
		to_tail = to_tail->to_tail;
		idx--;
	}
}

uint32_t* get_at_tail(List* list, size_t idx) {
	ListElement* to_head = list->tail;
	while(1) {
		if(to_head == NULL) {
			return NULL;
		}
		if(idx == 0) {
			return &to_head->value;
		}
		to_head = to_head->to_head;
		idx--;
	}
}

uint8_t is_symmetrical(List* list) {
	ListElement* to_head = list->tail;
	ListElement* to_tail = list->head;
	while(1) {
		if(to_head == NULL) {
			return 1;
		}
		if(to_head->value != to_tail->value) {
			return 0;
		}
		to_head = to_head->to_head;
		to_tail = to_tail->to_tail;
	}
}

int main(int argc, char** argv) {
	List list = make_list();

	while(1) {
		print_list(&list);
		char input[80];
		scanf("%s", input);
		if(strcmp(input, "pop_head") == 0) {
			uint32_t value;
			if(pop_head(&list, &value)) {
				printf("%d\n", value);
			} else {
				printf("List is empty\n");
			}
		} else if(strcmp(input, "pop_tail") == 0) {
			uint32_t value;
			if(pop_tail(&list, &value)) {
				printf("%d\n", value);
			} else {
				printf("List is empty\n");
			}
		} else if(strcmp(input, "get_at_head") == 0) {
			uint32_t idx;
			scanf("%d", &idx);
			uint32_t* value = get_at_head(&list, idx);
			if(value != NULL) {
				printf("%d\n", *value);
			} else {
				printf("Out of bounds\n");
			}
		} else if(strcmp(input, "get_at_tail") == 0) {
			uint32_t idx;
			scanf("%d", &idx);
			uint32_t* value = get_at_tail(&list, idx);
			if(value != NULL) {
				printf("%d\n", *value);
			} else {
				printf("Out of bounds\n");
			}
		} else if(strcmp(input, "add_head") == 0) {
			uint32_t value;
			scanf("%d", &value);
			add_head(&list, value);
		} else if(strcmp(input, "add_tail") == 0) {
			uint32_t value;
			scanf("%d", &value);
			add_tail(&list, value);
		} else if(strcmp(input, "dedupe") == 0) {
			deduplicate(&list);
		} else if(strcmp(input, "clear") == 0) {
			clear_list(&list);
		} else if(strcmp(input, "is_symm") == 0) {
			printf("%s\n", is_symmetrical(&list) ? "yes" : "no");
		} else if(strcmp(input, "exit") == 0) {
			break;
		} else {
			printf("Unknown command\n");
		}
	}
	
	clear_list(&list);

	return 0;
}
