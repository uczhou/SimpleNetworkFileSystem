#include <stdlib.h>
#include <string.h>
#include "utils.h"

void set_bitmap(Bitmap_t *map, int i) {

	int byte = i >> 3;

	map->bitmap[byte] |= 1 << (i & 7);

}

void unset_bitmap(Bitmap_t *map, int i) {

	int byte = i >> 3;

	map->bitmap[byte] &= ~(1 << (i & 7));
}

int get_bit_value(Bitmap_t *map, int i) {

	int byte = i >> 3;

	if (map->bitmap[byte] & (1 << (i & 7))) {
		return 1;
	}else{
		return 0;
	}

}

int get_empty_bit(Bitmap_t *map) {
	
	for (int i = 0; i < MAPSIZE; i++) {
		if(!get_bit_value(map, i)) {
			return i;
		}
	}

	return -1;
}
