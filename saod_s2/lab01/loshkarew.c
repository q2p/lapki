#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef struct {
	// Ёмкость таблицы
	size_t size;
	// Количество операций сравнений за время жизни
	uint64_t comparisons_count;
	// Количество коллизий за время жизни
	uint64_t collisions_count;
	// Количество операций вставки и поиска за время жизни
	uint64_t ops_count;
	// Ячейки таблицы, в которых хронятся строки.
	char** buckets;
} HashTable;

// Функция создаёт новую таблицу
HashTable hash_table_new(size_t size) {
	// Чтобы избежать головной боли, лишних проверок, устанавливаем минимальный размер таблицы в 1 элемент.
	if (size == 0) {
		size = 1;
	}

	HashTable ret;

	ret.comparisons_count = 0;
	ret.collisions_count = 0;
	ret.ops_count = 0;
	ret.size = size;
	// Выделяем память под нужное количество ячеек.
	ret.buckets = malloc(sizeof(char*) * size);
	// Записываем в ячейки нули. Это будет значить, что они свободны.
	memset(ret.buckets, 0, sizeof(char*) * size);

	return ret;
}

// Освобождает память после использования таблицы.
void hash_table_destroy(HashTable* set) {
	for(size_t i = 0; i < set->size; i++) {
		// Освобождаем элемент таблицы (строку).
		// ! Элемент может быть NULL, проверки не нужны !
		free(set->buckets[i]);
	}
		// Освобождаем сам контейнер ячеек.
		// ! Элемент может быть NULL, проверки не нужны !
	free(set->buckets);
}

// Добавляет элемент в таблицу.
uint8_t hash_table_add(HashTable* set, char* value) {
	// Определяем длину строки.
	size_t len = strlen(value);

	// Записываем операцию вставки.
	set->ops_count++;

	// Вычисляем хэш.
	uint32_t hash = value[0] + value[len-1];

	// Мы можем произвести рехеширование только `size` раз.
	for(size_t i = 0; i < set->size; i++) {
		// Рехеширование. Первый раз hash0 = hash.
		size_t idx = (hash + i) % set->size;

		char* bucket = set->buckets[idx];

		// Записываем операцию сравнения.
		set->comparisons_count++;
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
		set->comparisons_count++;
		// Если строка найдена в данной ячейке, то
		if(strcmp(bucket, value) == 0) {
			// Возвращаем 1. Строка уже есть в таблице.
			return 1;
		}

		// Место не найдено, записываем коллизию и пробуем ещё раз.
		set->collisions_count++;
	}
	
	// Возвращаем 0. Не нашли свободного места под новый элемент.
	return 0;
}

// Проверяем, есть ли элемент в таблице.
uint8_t hash_table_has(HashTable* set, char* value) {
	// Определяем длину строки.
	size_t len = strlen(value);

	// Записываем операцию вставки.
	set->ops_count++;

	// Вычисляем хэш.
	uint32_t hash = value[0] + value[len-1];

	// Мы можем произвести рехеширование только `size` раз.
	for(size_t i = 0; i < set->size; i++) {
		// Рехеширование. Первый раз hash0 = hash.
		size_t idx = (hash + i) % set->size;

		char* bucket = set->buckets[idx];

		// Записываем операцию сравнения.
		set->comparisons_count++;
		// Если ячейка пустая, то элемента нет в таблице. Возвращаем 0.
		if (bucket == NULL) {
			return 0;
		}
		
		// Записываем операцию сравнения.
		set->comparisons_count++;
		// Если ячейка хранит такую же строку, то мы нашли элемент в таблице. Возвращаем 1.
		if(strcmp(bucket, value) == 0) {
			return 1;
		}

		// Элемент не найден, записываем коллизию и пробуем следующий хэш.
		set->collisions_count++;
	}
	
	// Возвращаем 0. Не нашли элемент в таблице.
	return 0;
}

// Функция выводит содержимое таблицы.
void hash_table_print(HashTable* set) {
	// Перебираем все ячейки.
	for(size_t i = 0; i < set->size; i++) {
		char* bucket = set->buckets[i];
		// Если ячейка не пустая, то выводим её содержимое.
		if (bucket != NULL) {
			printf("%s\n", bucket);
		}
	}
}

void main() {
	// Просим ввести ёмкость таблицы.
	printf("Please enter hash set capacity...\n");
	size_t size;
	scanf("%ld", &size);

	// Создаём таблицу.
	HashTable table = hash_table_new(size);

	// Создаём буффер под комманды и элементы на стеке.
	char line[80];

	while(1) {
		// Читаем команду.
		scanf("%s", line);
		if (strcmp("add", line) == 0) { // Если комманда "add"
			// Читаем строку-элемент и пробуем добавить.
			scanf("%s", line);
			if (hash_table_add(&table, line)) {
				printf("Added %s to the table.\n", line);
			} else {
				printf("Error: Table is full.\n");
			}
		} else if (strcmp("find", line) == 0) { // Если комманда "find"
			// Читаем строку-элемент и пробуем найти.
			scanf("%s", line);
			if(hash_table_has(&table, line)) {
				printf("Value %s is present.\n", line);
			} else {
				printf("Value %s is absent.\n", line);
			}
		} else if (strcmp("list", line) == 0) { // Если комманда "list"
			// Выводим список элементов.
			printf("Table's contents:\n");
			hash_table_print(&table);
		} else if (strcmp("stats", line) == 0) { // Если комманда "stats"
			// Выводим среднее количество коллизий и сравнений на операцию.
			printf("Comparisons per operation: %.3f.\n", (float)table.comparisons_count / (float)table.ops_count);
			printf("Collisions per operation: %.3f.\n", (float)table.collisions_count / (float)table.ops_count);
		} else if (strcmp("exit", line) == 0) { // Если комманда "stats"
			// Прекращаем чтение, выходим из цикла.
			break;
		} else {
			// Неизвестная комманда
			printf("Unknown command. Try \"add\", \"find\", \"list\" or \"exit\".\n");
		}
	}

	// Уничтожаем таблицу.
	hash_table_destroy(&table);
}