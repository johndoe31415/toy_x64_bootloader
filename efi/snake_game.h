/*
	toy_x64_bootloader - Minimal bootloader for x86_64 using long mode and PML4
	Copyright (C) 2023-2023 Johannes Bauer

	This file is part of toy_x64_bootloader.

	toy_x64_bootloader is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; this program is ONLY licensed under
	version 3 of the License, later versions are explicitly excluded.

	toy_x64_bootloader is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with toy_x64_bootloader; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Johannes Bauer <JohannesBauer@gmx.de>
*/

#ifndef __SNAKE_GAME_H__
#define __SNAKE_GAME_H__

#include <stdint.h>
#include <stdbool.h>

#define FIELD_WIDTH			180
#define FIELD_HEIGHT		100
#define MAX_SNEK_LENGTH		200

enum playfield_item_t {
	EMPTY = 0,
	WALL = 1,
	SNEK = 2,
	PRECIOUS = 3,
};

enum direction_t {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

struct vec2_t {
	int x, y;
};

struct snake_game_t {
	unsigned int score;
	unsigned int pixel_width, pixel_height;
	unsigned int screen_offset_x, screen_offset_y;
	uint8_t playfield[FIELD_WIDTH * FIELD_HEIGHT];
	uint64_t rng;
	struct {
		unsigned int length;
		unsigned int speed;
		struct vec2_t head;
		struct {
			struct vec2_t pos[MAX_SNEK_LENGTH];
			unsigned int index;
			unsigned int length;
		} shape;
		enum direction_t direction;
		enum direction_t next_direction;
	} snek;
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
void snake_draw_pixel(struct snake_game_t *game, unsigned int x, unsigned int y);
void snake_game_draw_full(struct snake_game_t *game);
void snake_find_empty_pos(struct snake_game_t *game, struct vec2_t *vec);
void snake_game_init(struct snake_game_t *game, unsigned int screen_width, unsigned int screen_height, unsigned int screen_offset_x, unsigned int screen_offset_y);
bool snake_game_play(struct snake_game_t *game);
void gfx_draw_pixel(unsigned int x, unsigned int y, uint32_t pixel);
uint32_t kbd_readkey(void);
void snek_pos_dump(struct snake_game_t *game);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
