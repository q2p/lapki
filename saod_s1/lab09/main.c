#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

enum {
	MAP_SIZE_X = 64,
	MAP_SIZE_Y = 32,
	MAP_SIZE_XY = MAP_SIZE_X*MAP_SIZE_Y,
};

uint8_t MAP_CONTAINER[2*MAP_SIZE_XY] = { 0 };
uint8_t* MAP_1 = MAP_CONTAINER;
uint8_t* MAP_2 = MAP_CONTAINER+MAP_SIZE_XY;

static inline size_t idx(size_t x, size_t y) {
	return y*MAP_SIZE_X+x;
}

uint8_t neighbours(size_t x, size_t y) {
	uint8_t ret = 0;
	for(size_t sy = y-1; sy != y+2; sy++) {
		size_t ny = sy % MAP_SIZE_Y;
		for(size_t sx = x-1; sx != x+2; sx++) {
			size_t nx = sx % MAP_SIZE_X;

			if((nx != x | ny != y) & MAP_1[ny * MAP_SIZE_X + nx]) {
				ret++;
			}
		}
	}
	return ret;
}

void insert(uint8_t* object, size_t x, size_t y, size_t object_size_x, size_t object_size_y) {
	uint8_t neighbours = 0;
	for(size_t oy = 0; oy != object_size_y; oy++) {
		for(size_t ox = 0; ox != object_size_x; ox++) {
			MAP_1[idx(ox+x,oy+y)] |= object[oy*object_size_x+ox];
		}
	}
}

enum {
	GLIDER_X = 3,
	GLIDER_Y = 3,
	GROWER_X = 5,
	GROWER_Y = 5,
	DIEHARD_X = 8,
	DIEHARD_Y = 3,
};
uint8_t GLIDER[9] = {
	0, 1, 0,
	0, 0, 1,
	1, 1, 1,
};
uint8_t GROWER[25] = {
	1, 1, 1, 0, 1,
	1, 0, 0, 0, 0,
	0, 0, 0, 1, 1,
	0, 1, 1, 0, 1,
	1, 0, 1, 0, 1,
};
uint8_t DIEHARD[24] = {
	0, 0, 0, 0, 0, 0, 1, 0,
	1, 1, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 0, 1, 1, 1,
};

const struct timespec req = {
	tv_sec: 0,
	tv_nsec: 20000000L
};

clock_t next;

uint8_t* sus[2] = { "\033[0m.","\033[33m\033[1m#" };

int main(int argc, char** argv) {
	memset(MAP_1, 0, MAP_SIZE_XY);

	insert(GROWER, 32, 16, GROWER_X, GROWER_Y);
	// insert(GLIDER, 10, 10, GLIDER_X, GLIDER_Y);
	// insert(DIEHARD, 24, 24, DIEHARD_X, DIEHARD_Y);

	printf("\033[2J");
	while(1) {
		printf("\033[H");
		struct timespec rem;
		nanosleep(&req, &rem);

		for(size_t y = 0; y != MAP_SIZE_Y; y++) {
			for(size_t x = 0; x != MAP_SIZE_X; x++) {
				printf("%s", sus[MAP_1[idx(x, y)]]);
			}
			printf("\n");
		}

		for(size_t y = 0; y != MAP_SIZE_Y; y++) {
			for(size_t x = 0; x != MAP_SIZE_X; x++) {
				uint8_t n = neighbours(x, y);
				uint8_t a = MAP_1[idx(x, y)];
				a = n == 3 | (a & n == 2);
				MAP_2[idx(x, y)] = a;
			}
		}
		uint8_t* t = MAP_1;
		MAP_1 = MAP_2;
		MAP_2 = t;
	}
	return 0;
}