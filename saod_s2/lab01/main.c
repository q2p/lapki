#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef struct {
	// Ёмкость таблицы
	size_t capacity;
	// Количество операций сравнений за время жизни
	uint64_t stat_comparisons;
	// Количество коллизий за время жизни
	uint64_t stat_collisions;
	// Количество операций вставки и поиска за время жизни
	uint64_t stat_ops;
	// Ячейки таблицы, в которых хронятся строки.
	char** buckets;
} HashTable;

// Функция создаёт новую таблицу
HashTable hash_set_new(size_t capacity) {
	// Чтобы избежать головной боли, лишних проверок, устанавливаем минимальный размер таблицы в 1 элемент.
	if (capacity == 0) {
		capacity = 1;
	}

	HashTable ret;

	ret.stat_comparisons = 0;
	ret.stat_collisions = 0;
	ret.stat_ops = 0;
	ret.capacity = capacity;
	// Выделяем память под нужное количество ячеек.
	ret.buckets = malloc(sizeof(char*) * capacity);
	// Записываем в ячейки нули. Это будет значить, что они свободны.
	memset(ret.buckets, 0, sizeof(char*) * capacity);

	return ret;
}

// Освобождает память после использования таблицы.
void hash_set_destroy(HashTable* set) {
	for(size_t i = 0; i != set->capacity; i++) {
		// Освобождаем элемент таблицы (строку).
		// ! Элемент может быть NULL, проверки не нужны !
		free(set->buckets[i]);
	}
		// Освобождаем сам контейнер ячеек.
		// ! Элемент может быть NULL, проверки не нужны !
	free(set->buckets);
}

// Добавляет элемент в таблицу.
uint8_t hash_set_add(HashTable* set, char* value) {
	// Определяем длину строки.
	size_t len = strlen(value);

	// Записываем операцию вставки.
	set->stat_ops++;

	// Вычисляем хэш.
	uint32_t hash = value[0] + value[len-1];

	// Мы можем произвести рехеширование только `capacity` раз.
	for(size_t i = 0; i < set->capacity; i++) {
		// Рехеширование. Первый раз hash0 = hash.
		size_t idx = (hash + i) % set->capacity;

		char* bucket = set->buckets[idx];

		// Записываем операцию сравнения.
		set->stat_comparisons++;
		// Если ячейка пустая, то
		if (bucket == NULL) {
			// Выделяем память в куче под строку. +1 нужен для терминирующего нуля '\0'.
			bucket = malloc(len + 1);
			// Копируем строку из стека в кучу.
			memcpy(bucket, value, len + 1);
			// Записываем указатель на новую строку в куче.
			set->buckets[idx] = bucket;
			// Возвращаем 1. Строка добавлена.
			return 1;
		}

		// Записываем операцию сравнения.
		set->stat_comparisons++;
		// Если строка найдена в данной ячейке, то
		if(strcmp(bucket, value) == 0) {
			// Возвращаем 1. Строка уже есть в таблице.
			return 1;
		}

		// Место не найдено, записываем коллизию и пробуем ещё раз.
		set->stat_collisions++;
	}
	
	// Возвращаем 0. Не нашли свободного места под новый элемент.
	return 0;
}

// Проверяем, есть ли элемент в таблице.
uint8_t hash_set_has(HashTable* set, char* value) {
	// Определяем длину строки.
	size_t len = strlen(value);

	// Записываем операцию вставки.
	set->stat_ops++;

	// Вычисляем хэш.
	uint32_t hash = value[0] + value[len-1];

	// Мы можем произвести рехеширование только `capacity` раз.
	for(size_t i = 0; i < set->capacity; i++) {
		// Рехеширование. Первый раз hash0 = hash.
		size_t idx = (hash + i) % set->capacity;

		char* bucket = set->buckets[idx];

		// Записываем операцию сравнения.
		set->stat_comparisons++;
		// Если ячейка пустая, то элемента нет в таблице. Возвращаем 0.
		if (bucket == NULL) {
			return 0;
		}
		
		// Записываем операцию сравнения.
		set->stat_comparisons++;
		// Если ячейка хранит такую же строку, то мы нашли элемент в таблице. Возвращаем 1.
		if(strcmp(bucket, value) == 0) {
			return 1;
		}

		// Элемент не найден, записываем коллизию и пробуем следующий хэш.
		set->stat_collisions++;
	}
	
	// Возвращаем 0. Не нашли элемент в таблице.
	return 0;
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

void main() {
	// Просим ввести ёмкость таблицы.
	printf("Please enter hash set capacity...\n");
	size_t size;
	scanf("%ld", &size);

	// Создаём таблицу.
	HashTable set = hash_set_new(size);

	// Создаём буффер под комманды и элементы на стеке.
	char line[80];

	while(1) {
		// Читаем команду.
		scanf("%s", line);
		if (strcmp("add", line) == 0) { // Если комманда "add"
			// Читаем строку-элемент и пробуем добавить.
			scanf("%s", line);
			if (hash_set_add(&set, line)) {
				printf("Added %s to the table.", line);
			} else {
				printf("Error: Table is full.");
			}
		} else if (strcmp("find", line) == 0) { // Если комманда "find"
			// Читаем строку-элемент и пробуем найти.
			scanf("%s", line);
			if(hash_set_has(&set, line)) {
				printf("Value %s is present.", line);
			} else {
				printf("Value %s is absent.", line);
			}
		} else if (strcmp("list", line) == 0) { // Если комманда "list"
			// Выводим список элементов.
			printf("Table's contents:");
			hash_set_print(&set);
		} else if (strcmp("stats", line) == 0) { // Если комманда "stats"
			// Выводим среднее количество коллизий и сравнений на операцию.
			printf("Comparisons per operation: %.3f.\n", (float)set.stat_comparisons / (float)set.stat_ops);
			printf("Collisions per operation: %.3f.\n", (float)set.stat_collisions / (float)set.stat_ops);
		} else if (strcmp("exit", line) == 0) { // Если комманда "stats"
			// Прекращаем чтение, выходим из цикла.
			break;
		} else {
			// Неизвестная комманда
			printf("Unknown command. Try \"add\", \"find\", \"list\" or \"exit\".");
		}
	}

	// Уничтожаем таблицу.
	hash_set_destroy(&set);
}