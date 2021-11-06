#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef struct Element {
	char* content;
	struct Element* next;
} Element;

typedef struct {
	int size;
	int stat_comparisons;
	int stat_ops;
	Element** buckets;
} HashSet;

HashSet hash_set_new(int size) {
	HashSet ret;
	ret.size = size;
	ret.stat_comparisons = 0;
	ret.stat_ops = 0;
	ret.buckets = (Element**) malloc(sizeof(Element*) * size);
	for (int i = 0; i < size; i++) {
		ret.buckets[i] = NULL;
	}
	return ret;
}

void hash_set_print(HashSet* set) {
	for(int i = 0; i != set->size; i++) {
		Element* bucket = set->buckets[i];
		while (bucket != NULL) {
			printf("%s\n", bucket->content);
			bucket = bucket->next;
		}
	}
}

void hash_set_destroy(HashSet* set) {
	for(int i = 0; i != set->size; i++) {
		Element* bucket = set->buckets[i];
		while (bucket != NULL) {
			Element* next = bucket->next;
			free(bucket);
			bucket = next;
		}
	}
	free(set->buckets);
}

void hash_set_add(HashSet* set, char* identifier) {
	set->stat_ops++;

	int len = strlen(identifier);

	int idx = identifier[len-1] % set->size;

	char* new_identifier = malloc(len+1);

	strcpy(new_identifier, identifier);

	Element* old = set->buckets[idx];

	Element* new = (Element*) malloc(sizeof(Element));

	new->next = old;
	new->content = new_identifier;

	set->buckets[idx] = new;
}

char hash_set_has(HashSet* restrict set, char* identifier) {
	set->stat_ops++;

	int len = strlen(identifier);

	int idx = identifier[len-1] % set->size;
	
	Element* bucket = set->buckets[idx];

	while (bucket != NULL) {
		set->stat_comparisons++;
		if(strcmp(bucket->content, identifier) == 0) {
			return 1;
		}
		bucket = bucket->next;
	}
	
	return 0;
}

void main() {
	puts("Please enter hash set capacity...");

	int size;

	scanf("%d", &size);

	HashSet set = hash_set_new(size);

	char line[80];

	while(1) {
		scanf("%s", line);
		if (strcmp("add", line) == 0) {
			scanf("%s", line);
			hash_set_add(&set, line);
		} else if (strcmp("find", line) == 0) {
			scanf("%s", line);
			if(hash_set_has(&set, line)) {
				puts("Found.");
			} else {
				puts("Not Found.");
			}
		} else if (strcmp("list", line) == 0) {
			puts("List of identifiers:");
			hash_set_print(&set);
		} else if (strcmp("stats", line) == 0) {
			printf("Comparisons per operation: %.3f.\n", (float)set.stat_comparisons / (float)set.stat_ops);
		} else {
			puts("Error.");
		}
	}
	hash_set_destroy(&set);
}