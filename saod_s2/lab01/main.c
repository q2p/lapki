#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef struct {
	uint64_t x;
	uint64_t w;
	uint64_t s;
} MSWS;

void msws_tick(MSWS* restrict msws) {
	msws->w += msws->s;
	msws->x = msws->x * msws->x + msws->w;
	msws->x = (msws->x >> 32) | (msws->x << 32);
}

MSWS msws_new(uint64_t seed) {
	MSWS ret;
	ret.x = 0;
	ret.w = 0;
	ret.s = (seed << 1) + 0xb5ad4eceda1ce2a9;
	msws_tick(&ret);
	return ret;
}
uint32_t msws_next(MSWS* restrict msws) {
	msws_tick(msws);
	return msws->x;
}

typedef struct {
	size_t capacity;
	size_t available;
	uint64_t stat_comparisons;
	uint64_t stat_collisions;
	uint64_t stat_ops;
	uint64_t max_hops;
	char** buckets;
} HashSet;

HashSet hash_set_new(size_t capacity) {
	if (capacity == 0) {
		capacity = 1;
	}
	HashSet ret;
	ret.stat_comparisons = 0;
	ret.stat_collisions = 0;
	ret.stat_ops = 0;
	ret.capacity = capacity;
	ret.available = capacity;
	ret.max_hops = 0;
	ret.buckets = malloc(sizeof(char*) * capacity);
	memset(ret.buckets, 0, sizeof(char*) * capacity);
	return ret;
}

void hash_set_destroy(HashSet* restrict set) {
	for(size_t i = 0; i != set->capacity; i++) {
		free(set->buckets[i]);
	}
}

uint8_t hash_set_add(HashSet* restrict set, char* restrict identifier, size_t identifier_len) {
	if(set->available == 0) {
		return 1;
	}

	MSWS msws = msws_new(812762606);

	set->stat_ops++;

	uint32_t hash = identifier[0] + identifier[identifier_len-1];

	uint64_t hops = 0;

	while(1) {
		hops++;

		size_t idx = hash % set->capacity;

		char* bucket = set->buckets[idx];

		if (bucket == NULL) {
			bucket = malloc(identifier_len + 1);
			memcpy(bucket, identifier, identifier_len + 1);
			set->buckets[idx] = bucket;
			set->available--;
			if (hops > set->max_hops) {
				set->max_hops = hops;
			}
			return 0;
		}

		set->stat_comparisons++;
		if(strcmp(bucket, identifier) == 0) {
			return 1;
		}
		set->stat_collisions++;

		hash ^= msws_next(&msws);
	}
}

uint8_t hash_set_has(HashSet* restrict set, char* restrict identifier, size_t identifier_len) {
	MSWS msws = msws_new(812762606);

	set->stat_ops++;
	
	uint32_t hash = identifier[0] + identifier[identifier_len-1];

	for(uint64_t i = set->max_hops; i != 0; i--) {
		size_t idx = hash % set->capacity;

		char* bucket = set->buckets[idx];

		if (bucket == NULL) {
			return 0;
		}
		
		set->stat_comparisons++;
		if(strcmp(bucket, identifier) == 0) {
			return 1;
		}
		set->stat_collisions++;

		hash ^= msws_next(&msws);
	}
	
	return 0;
}

void main() {
	puts("Please enter hash set capacity...");

	size_t capacity;

	scanf("%ld", &capacity);

	HashSet set = hash_set_new(capacity);

	char command;
	char* line = NULL;
	size_t buff_len = 0;
	size_t line_length;

	getline(&line, &buff_len, stdin);

	while(1) {
		line_length = getline(&line, &buff_len, stdin);
		line[--line_length] = '\0';
		if (strcmp("a", line) == 0) {
			line_length = getline(&line, &buff_len, stdin);
			line[--line_length] = '\0';
			if(hash_set_add(&set, line, line_length)) {
				printf("Failed to insert \"%s\". Hash set is full.\n", line);
			} else {
				printf("Inserted \"%s\".\n", line);
			}
		} else if (strcmp("q", line) == 0) {
			line_length = getline(&line, &buff_len, stdin);
			line[--line_length] = '\0';
			if(hash_set_has(&set, line, line_length)) {
				puts("Identifier is present.");
			} else {
				puts("Identifier is absent.");
			}
			hash_set_has(&set, line, line_length);
		} else if (strcmp("l", line) == 0) {
			puts("List of identifiers:");
			for(size_t i = 0; i != set.capacity; i++) {
				char* identifier = set.buckets[i];
				if (identifier != NULL) {
					puts(identifier);
				}
			}
		} else if (strcmp("s", line) == 0) {
			printf("Avg collisions: %.3f.\n", (float)set.stat_collisions / (float)set.stat_ops);
			printf("Avg comparisons: %.3f.\n", (float)set.stat_comparisons / (float)set.stat_ops);
		} else if (strcmp("e", line) == 0) {
			break;
		} else {
			puts("Unknown command. Try \"a\" to add, \"q\" to query, \"l\" to list, \"s\" for stats and \"e\" to exit.");
		}
	}
	free(line);
	hash_set_destroy(&set);
}