#include <stdlib.h>
#include <stdio.h>

typedef struct _LinkedElement LinkedElement;
struct _LinkedElement {
	int value;
	LinkedElement* next;
};

LinkedElement* add_first(LinkedElement* head, LinkedElement* new_element) {
	new_element->next = head;
	return new_element;
}

LinkedElement* add_last(LinkedElement* head, LinkedElement* new_element) {
	if (head == NULL) {
		return new_element;
	}
	while(head->next != NULL) {
		head = head->next;
	}
	head->next = new_element;
	return head;
}

LinkedElement* insert_sorted(LinkedElement* head, LinkedElement* new_element) {
	LinkedElement* prev = NULL;
	LinkedElement* next = head;
	while(next != NULL) {
		if (new_element->value == next->value) {
			free(new_element);
			return head;
		}
		if (new_element->value < next->value) {
			break;
		}
		prev = next;
		next = next->next;
	}
	new_element->next = next;
	if (prev != NULL) {
		prev->next = new_element;
		return head;
	} else {
		return new_element;
	}
}

void free_list(LinkedElement* head) {
	while(head != NULL) {
		LinkedElement* current = head;
		head = head->next;
		free(current);
	}
}

void print_list(LinkedElement* head) {
	while(head != NULL) {
		printf("%d ", head->value);
		head = head->next;
	}
	printf("\n");
}

int main(int argc, char** argv) {
	LinkedElement* head = NULL;

	while(1) {
		int input;
		scanf("%d", &input);
		if (input == 0) {
			break;
		}

		LinkedElement* element = (LinkedElement*) malloc(sizeof(LinkedElement));
		element->value = input;

		head = insert_sorted(head, element);

		print_list(head);
	}

	print_list(head);

	free_list(head);
	
	return 0;
}
