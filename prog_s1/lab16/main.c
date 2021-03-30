#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define BUFFER_LENGTH (1048576)

typedef struct _StackElement StackElement;
struct _StackElement {
	int value;
	StackElement* next;
};

void push_stack(StackElement** stack, int value) {
	StackElement* new_element = (StackElement*) malloc(sizeof(StackElement));
	new_element->value = value;
	new_element->next = *stack;
	*stack = new_element;
}

int pop_stack(StackElement** stack) {
	if(*stack == NULL) {
		return -1;
	}
	StackElement* current = *stack;
	int ret = (*stack)->value;
	*stack = (*stack)->next;
	free(current);
	return ret;
}

int pop_stack_N(StackElement** stack, size_t amount) {
	int* ret = (int*) malloc(sizeof(int)*amount);
	for(size_t i = 0; i != amount; i++) {
		StackElement* current = *stack;
		ret[i] = (*stack)->value;
		*stack = (*stack)->next;
		free(current);
	}
}

char is_stack_empty(StackElement* stack) {
	return stack == NULL;
}

size_t get_stack_length(StackElement* stack) {
	size_t i = 0;
	while(stack != NULL) {
		stack = stack->next;
		i++;
	}
	return i;
}

void free_stack(StackElement* stack) {
	while(stack != NULL) {
		StackElement* current = stack;
		stack = stack->next;
		free(current);
	}
}

/// Имплементация, через стек
char check_expression_stack(char* expr) {
	StackElement* stack = NULL;
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

/// Более эффективная имплементация, чем стек
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

/// Генерирует большое валидное выражение
void generate_expression(char* expr) {
	size_t half = ((BUFFER_LENGTH-1)/2);
	size_t i = 0;
	while(i != half) {
		expr[i] = '(';
		i++;
	}
	while(i != 2*half) {
		expr[i] = ')';
		i++;
	}
	expr[i] = '\0';
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
