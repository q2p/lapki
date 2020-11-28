#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define BUFFER_LENGTH (1048576)

typedef struct _LinkedElement LinkedElement;
struct _LinkedElement {
	int value;
	LinkedElement* next;
};

void push_stack(LinkedElement** stack, int value) {
	LinkedElement* new_element = (LinkedElement*) malloc(sizeof(LinkedElement));
	new_element->value = value;
	new_element->next = *stack;
	*stack = new_element;
}

int pop_stack(LinkedElement** stack) {
	if(*stack == NULL) {
		return -1;
	}
	LinkedElement* current = *stack;
	int ret = (*stack)->value;
	*stack = (*stack)->next;
	free(current);
	return ret;
}

int pop_stack_N(LinkedElement** stack, size_t amount) {
	int* ret = (int*) malloc(sizeof(int)*amount);
	for(size_t i = 0; i != amount; i++) {
		LinkedElement* current = *stack;
		ret[i] = (*stack)->value;
		*stack = (*stack)->next;
		free(current);
	}
}

char is_stack_empty(LinkedElement* stack) {
	return stack == NULL;
}

size_t get_stack_length(LinkedElement* stack) {
	size_t i = 0;
	while(stack != NULL) {
		stack = stack->next;
		i++;
	}
	return i;
}

void free_stack(LinkedElement* stack) {
	while(stack != NULL) {
		LinkedElement* current = stack;
		stack = stack->next;
		free(current);
	}
}

char check_expression_stack(char* expr) {
	LinkedElement* stack = NULL;
	while(*expr != '\0') {
		switch (*expr) {
			case '(':
				push_stack(&stack, 0);
				break;
			case ')':
				if (pop_stack(&stack) == -1) {
					return 0;
				}
				break;
			default:
				break;
		}
		expr++;
	}
	if (is_stack_empty(stack)) {
		return 1;
	} else {
		free_stack(stack);
		return 0;
	}
}

char check_expression_fast(char* expr) {
	size_t c = 0;
	while(*expr != '\0') {
		switch (*expr) {
			case '(':
				c++;
				break;
			case ')':
				if (c == 0) {
					return 0;
				}
				c--;
				break;
			default:
				break;
		}
		expr++;
	}
	return c == 0;
}

void generate_expression(char* expr) {
	size_t c = 0;
	size_t i = 0;
	while(1) {
		if (i + c < BUFFER_LENGTH - 1) {
			if ((c == 0) || (rand() & 0x1)) {
				expr[i] = '(';
				c++;
			} else {
				expr[i] = ')';
				c--;
			}
		} else {
			if (c == 0) {
				expr[i] = '\0';
				break;
			} else {
				expr[i] = ')';
				c--;
			}
		}
		i++;
	}
}

int main(int argc, char** argv) {
	char string[BUFFER_LENGTH];

	generate_expression(string);
	printf("Generated Expression: %s\n", string);

	// Замеряем скорость имплементации через стэк
	clock_t start = clock();
	if (check_expression_stack(string)) {
		puts("OK");
	} else {
		puts("BAD");
	}
	printf("Stack based took %lld ticks\n", (long long)difftime(clock(), start));

	// Замеряем скорость имплементации через счётчик
	start = clock();
	if (check_expression_fast(string)) {
		puts("OK");
	} else {
		puts("BAD");
	}
	printf("Counter based took %lld ticks\n", (long long)difftime(clock(), start));

	// Начинаем проверку выражений
	puts("Enter an expression: ");
	while(1) {
		fgets(string, BUFFER_LENGTH, stdin);

		if (check_expression_stack(string)) {
			puts("OK");
		} else {
			puts("BAD");
		}
	}
	
	return 0;
}
