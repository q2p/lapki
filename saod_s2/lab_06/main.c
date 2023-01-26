#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <float.h>

enum {
	MAX_CITIES = 128,
	MAX_POINTS = 2*MAX_CITIES,
	MATRIX_SIZE = MAX_CITIES*MAX_CITIES,
};

float distances[MATRIX_SIZE];
uint8_t visited[MATRIX_SIZE];
uint8_t current[MAX_CITIES];
uint8_t best[MAX_CITIES];
float best_dist = FLT_MAX;
uint8_t visited_count = 0;
size_t cities_count = 0;
 
void tsp(size_t city_idx, float cost) {
	visited[city_idx] = 1;
	current[visited_count] = city_idx;
	visited_count++;

	if (visited_count == 2) {
		printf("Done: %2d/%2d\n", city_idx, cities_count);
	}

	if (visited_count == cities_count) {
		cost += distances[city_idx*MAX_CITIES+0];
		if (cost < best_dist) {
			best_dist = cost;
			memcpy(best, current, MAX_CITIES);
		}
	} else {
		for (size_t i = 1; i != cities_count; i++) {
			if (!visited[i]) {
				tsp(i, cost + (uint32_t)distances[city_idx*MAX_CITIES+i]);
			}
		}
	}
	visited[city_idx] = 0;
	visited_count--;
};

int main() {
	FILE* file = fopen("cities.txt", "r");

	uint8_t cities[MAX_POINTS];		
	while (cities_count != MAX_POINTS) {
		int x, y;
		if (fscanf(file, "%d%d", &x, &y) != 2) {
			break;
		}
		cities[cities_count++] = x;
		cities[cities_count++] = y;
	}
	fclose(file);

	cities_count /= 2;

	memset(visited, 0, MAX_CITIES);

	for(size_t y = 0; y != cities_count; y++) {
		for(size_t x = 0; x != cities_count; x++) {
			int8_t dx = (int8_t)cities[2*x+0] - (int8_t)cities[2*y+0];
			int8_t dy = (int8_t)cities[2*x+1] - (int8_t)cities[2*y+1];
			float dd = (float)sqrt(((int16_t)dx*(int16_t)dx + (int16_t)dy*(int16_t)dy));
			distances[y*MAX_CITIES+x] = dd;
		}
	}
	
	tsp(0, 0);
	
	printf("Best path:\n");
	for (size_t i = 0; i != cities_count; i++) {
		printf("%2d %2d\n", cities[2*best[i]+0],cities[2*best[i]+1]);
	}
	printf("%2d %2d\n", cities[0], cities[1]);
	printf("Total Distance: %f\n", best_dist);
	
	return 0;
}