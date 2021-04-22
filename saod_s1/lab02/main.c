#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
	void* ptr;
	size_t len;
	size_t capacity;
} Vec;

Vec vec_new(size_t starting_capacity) {
	Vec s;
	if(starting_capacity != 0) {
		s.ptr = malloc(starting_capacity);
	} else {
		s.ptr = NULL;
	}
	s.len = 0;
	s.capacity = starting_capacity;
	return s;
}

void vec_free(Vec* vec) {
	if(vec->ptr) {
		free(vec->ptr);
	}
}

void vec_reserve(Vec* vec, size_t additional) {
	size_t req = vec->len + additional;
	if (req <= vec->capacity) {
		return;
	}
	vec->capacity = req + req / 2;
	if(vec->ptr) {
		vec->ptr = realloc(vec->ptr, vec->capacity);
	} else {
		vec->ptr = malloc(vec->capacity);
	}
}

void* vec_next(Vec* vec, size_t len) {
	vec_reserve(vec, len);
	void* ret = (void*)(((uint8_t*) vec->ptr)+vec->len);
	vec->len += len;
	return ret;
}

typedef struct {
	uint32_t uid;
	char name[128];
	char group[12];
	uint8_t average_grade;
} Student;

void print_student(Student* student) {
	printf("%d %s %s %d\n", student->uid, student->group, student->name, student->average_grade);
}

uint8_t is_whitespace(char c) {
	switch (c) {
		case '\t':
		case '\n':
		case '\v':
		case '\f':
		case '\r':
		case ' ':
			return 1;
		default:
			return 0;
	}
}

void trim(char *str) {
	char* ns = str;
	while(1) {
		if(*ns == '\0') {
			*str = '\0';
			return;
		}
		if(!is_whitespace(*ns)) {
			break;
		}
		ns++;
	}
	char* c = ns + 1;
	char* end = c;
	while(*c != '\0') {
		if(!is_whitespace(*c)) {
			end = c + 1;
		}
		c++;
	}
	size_t nlen = end - ns;
	memmove(str, ns, nlen);
	str[nlen] = '\0';
}

int main(int argc, char** argv) {
	if (argc != 3) {
		perror("Too many or too little arguments.");
		return 1;
	}

	FILE* file = fopen("./students.txt", "r");

	if(file == NULL) {
		perror("Can't open file.");
		return 1;
	}
	
	size_t length;
	fscanf(file, "%ld", &length);
	const size_t target_len = sizeof(Student) * length;
	Vec list = vec_new(target_len);
	for(size_t i = 0; i != length; i++) {
		Student* next = vec_next(&list, sizeof(Student));
		fscanf(file, "%d%s%hhd", &next->uid, next->group, &next->average_grade);
		fgets(next->name, 127, file);
		trim(next->name);
	}

	if (strcmp("-z", argv[1]) == 0) {
		uint32_t uid = atoi(argv[2]);
		for(size_t i = 0; i != length; i++) {
			Student* student = ((Student*)list.ptr) + i;
			if(student->uid == uid) {
				print_student(student);
			}
		}
	} else if (strcmp("-n", argv[1]) == 0) {
		for(size_t i = 0; i != length; i++) {
			Student* student = ((Student*)list.ptr) + i;
			if(strstr(student->name, argv[2])) {
				print_student(student);
			}
		}
	} else if (strcmp("-g", argv[1]) == 0) {
		for(size_t i = 0; i != length; i++) {
			Student* student = ((Student*)list.ptr) + i;
			if(strcmp(student->group, argv[2]) == 0) {
				print_student(student);
			}
		}
	} else if (strcmp("-b", argv[1]) == 0) {
		uint8_t grade = atoi(argv[2]);
		for(size_t i = 0; i != length; i++) {
			Student* student = ((Student*)list.ptr) + i;
			if(student->average_grade >= grade) {
				print_student(student);
			}
		}
	} else {
		fprintf(stderr, "Illegal argument %s.", argv[1]);
		return 1;
	}

	fclose(file);
	vec_free(&list);
	return 0;
}
