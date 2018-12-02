#ifndef __UTILS_H__
#define __UTILS_H__

#define MAPSIZE 4096

typedef struct __Bitmap_t {
	unsigned char bitmap[MAPSIZE/8];
}Bitmap_t;

void set_bitmap(Bitmap_t *map, int i);

void unset_bitmap(Bitmap_t *map, int i);

int get_bit_value(Bitmap_t *map, int i);

int get_empty_bit(Bitmap_t *map);

#endif