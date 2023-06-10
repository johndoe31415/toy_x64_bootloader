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

#include <string.h>
#include <stdint.h>
#include "snake_game.h"
#include "snake_gfx.h"
#include "snake_kbd.h"
#include "snake_font.h"
#include "vcr-osd-mono-20.h"
#include "snake_timer.h"

static uint64_t rdtsc(void) {
	uint64_t value;
	__asm__ __volatile__ (
		"xor %%rax, %%rax"			"\n\t"
		"cpuid"						"\n\t"
		"rdtsc"						"\n\t"
	: "=A" (value)
	:
	: "rbx", "rcx", "rdx"
	);
	return value;
}

static const uint32_t palette[] = {
	[EMPTY] = 0,
	[WALL] = 0x00e74c3c,
	[SNEK] = 0x003498db,
	[PRECIOUS] = 0x0027ae60,
};

static void snake_playfield_set(struct snake_game_t *game, unsigned int x, unsigned int y, enum playfield_item_t item) {
	game->playfield[x + (FIELD_WIDTH * y)] = item;
}

static enum playfield_item_t snake_playfield_get(struct snake_game_t *game, unsigned int x, unsigned int y) {
	return game->playfield[x + (FIELD_WIDTH * y)];
}

void snake_draw_pixel(struct snake_game_t *game, unsigned int x, unsigned int y) {
	enum playfield_item_t item = snake_playfield_get(game, x, y);
	uint32_t pixel = palette[item];
	unsigned int offsetx = game->screen_offset_x + x * game->pixel_width;
	unsigned int offsety = game->screen_offset_y + y * game->pixel_height;
	for (unsigned int py = 0; py < game->pixel_height; py++) {
		for (unsigned int px = 0; px < game->pixel_width; px++) {
			gfx_draw_pixel(offsetx + px, offsety + py, pixel);
		}
	}
}

void snake_game_draw_full(struct snake_game_t *game) {
	gfx_fill_screen(COLOR_BLACK);
	for (unsigned int y = 0; y < FIELD_HEIGHT; y++) {
		for (unsigned int x = 0; x < FIELD_WIDTH; x++) {
			snake_draw_pixel(game, x, y);
		}
	}
}

static void snake_playfield_set_horizontal(struct snake_game_t *game, unsigned int x_start, unsigned int y, unsigned int length, enum playfield_item_t item) {
	for (unsigned int x = x_start; x < x_start + length; x++) {
		snake_playfield_set(game, x, y, item);
	}
}

static void snake_playfield_set_vertical(struct snake_game_t *game, unsigned int x, unsigned int y_start, unsigned int length, enum playfield_item_t item) {
	for (unsigned int y = y_start; y < y_start + length; y++) {
		snake_playfield_set(game, x, y, item);
	}
}

static uint64_t snake_xorshift_rng(struct snake_game_t *game) {
	game->rng ^= game->rng << 13;
	game->rng ^= game->rng >> 7;
	game->rng ^= game->rng << 17;
	return game->rng;
}

static void snake_randomize(struct snake_game_t *game, uint64_t new_entropy) {
	game->rng ^= new_entropy;
	snake_xorshift_rng(game);
}

void snake_find_empty_pos(struct snake_game_t *game, struct vec2_t *vec) {
	while (true) {
		vec->x = snake_xorshift_rng(game) % FIELD_WIDTH;
		vec->y = snake_xorshift_rng(game) % FIELD_HEIGHT;
		enum playfield_item_t item = snake_playfield_get(game, vec->x, vec->y);
		if (item == EMPTY) {
			return;
		}
	}
}

static struct vec2_t snake_place_precious(struct snake_game_t *game) {
	struct vec2_t pos;
	snake_find_empty_pos(game, &pos);
	snake_playfield_set(game, pos.x, pos.y, PRECIOUS);
	return pos;
}

static void snake_clear_status_bar(void) {
	gfx_fill(100, 0, 500, 35, COLOR_BLACK);
}

void snake_game_init(struct snake_game_t *game, unsigned int screen_width, unsigned int screen_height, unsigned int screen_offset_x, unsigned int screen_offset_y) {
	memset(game, 0, sizeof(*game));

	game->screen_offset_x = screen_offset_x;
	game->screen_offset_y = screen_offset_y;
	game->pixel_width = screen_width / FIELD_WIDTH;
	game->pixel_height = screen_height / FIELD_HEIGHT;

	/* Init playfield border */
	snake_playfield_set_horizontal(game, 0, 0, FIELD_WIDTH, WALL);
	snake_playfield_set_horizontal(game, 0, FIELD_HEIGHT - 1, FIELD_WIDTH, WALL);
	snake_playfield_set_vertical(game, 0, 0, FIELD_HEIGHT, WALL);
	snake_playfield_set_vertical(game, FIELD_WIDTH - 1, 0, FIELD_HEIGHT, WALL);

	/* Draw string "U" */
	const unsigned int logox = 40;
	const unsigned int logoy = 26;
	snake_playfield_set_vertical(game, logox, logoy, 50, WALL);
	snake_playfield_set_horizontal(game, logox, logoy + 50, 20, WALL);
	snake_playfield_set_vertical(game, logox + 19, logoy, 50, WALL);

	/* Draw string "E" */
	snake_playfield_set_vertical(game, logox + 30, logoy, 50, WALL);
	snake_playfield_set_horizontal(game, logox + 30, logoy, 20, WALL);
	snake_playfield_set_horizontal(game, logox + 30, logoy + 25, 20, WALL);
	snake_playfield_set_horizontal(game, logox + 30, logoy + 50, 20, WALL);

	/* Draw string "F" */
	snake_playfield_set_vertical(game, logox + 60, logoy, 51, WALL);
	snake_playfield_set_horizontal(game, logox + 60, logoy, 20, WALL);
	snake_playfield_set_horizontal(game, logox + 60, logoy + 25, 20, WALL);

	/* Draw string "I" */
	snake_playfield_set_vertical(game, logox + 90, logoy, 51, WALL);

	/* Init player */
	game->snek.length = 3;
	game->snek.speed = 1;
	game->snek.head.x = 50;
	game->snek.head.y = 13;
	game->snek.next_direction = RIGHT;

	/* Init RNG statically */
	game->rng = (uint64_t)0xfe9efb9e24898078;

	/* Put first precious */
	snake_place_precious(game);

	snake_game_draw_full(game);
}

static struct vec2_t *snake_shape_ptr(struct snake_game_t *game, int index) {
	unsigned int aindex = (MAX_SNEK_LENGTH + game->snek.shape.index - game->snek.shape.length + index) % MAX_SNEK_LENGTH;
	struct vec2_t *element = game->snek.shape.pos + aindex;
	return element;
}

static void snake_shape_append(struct snake_game_t *game) {
	if (game->snek.shape.length >= MAX_SNEK_LENGTH) {
		/* Should never happen! */
		return;
	}
	struct vec2_t *new_pos = snake_shape_ptr(game, game->snek.shape.length);
	new_pos->x = game->snek.head.x;
	new_pos->y = game->snek.head.y;
	game->snek.shape.length += 1;
	game->snek.shape.index = (game->snek.shape.index + 1) % MAX_SNEK_LENGTH;
}

static struct vec2_t *snake_shape_remove(struct snake_game_t *game) {
	game->snek.shape.length -= 1;
	return snake_shape_ptr(game, -1);
}

static void snake_game_print_score(struct snake_game_t *game) {
	snake_clear_status_bar();
	struct cursor_t cursor = {
		.x = 100,
		.y = 30,
	};
	font_printf(&font_vcr_osd_mono_20, &cursor, 0xffffff, 0, "Score: %-5d", game->score);
}

static bool snake_game_tick(struct snake_game_t *game) {
	game->snek.direction = game->snek.next_direction;
	if (game->snek.direction == RIGHT) {
		game->snek.head.x = (game->snek.head.x + 1) % FIELD_WIDTH;
	} else if (game->snek.direction == LEFT) {
		game->snek.head.x = (game->snek.head.x + FIELD_WIDTH - 1) % FIELD_WIDTH;
	} else if (game->snek.direction == DOWN) {
		game->snek.head.y = (game->snek.head.y + 1) % FIELD_HEIGHT;
	} else if (game->snek.direction == UP) {
		game->snek.head.y = (game->snek.head.y + FIELD_HEIGHT - 1) % FIELD_HEIGHT;
	}

	enum playfield_item_t item = snake_playfield_get(game, game->snek.head.x, game->snek.head.y);
	if ((item == SNEK) || (item == WALL)) {
		/* snek ded */
		return false;
	} else if (item == PRECIOUS) {
		/* snek food */
		game->score += game->snek.length;
		struct vec2_t precious = snake_place_precious(game);
		snake_draw_pixel(game, precious.x, precious.y);
		if (game->snek.length < MAX_SNEK_LENGTH - 1) {
			game->snek.length++;
		}
		snake_game_print_score(game);
	}


	snake_playfield_set(game, game->snek.head.x, game->snek.head.y, SNEK);
	snake_draw_pixel(game, game->snek.head.x, game->snek.head.y);

	snake_shape_append(game);
	if (game->snek.shape.length > game->snek.length) {
		struct vec2_t *removed = snake_shape_remove(game);
		snake_playfield_set(game, removed->x, removed->y, EMPTY);
		snake_draw_pixel(game, removed->x, removed->y);
	}

	return true;
}

static void snake_read_keyboard(struct snake_game_t *game) {
	uint32_t next_char;
	while ((next_char = kbd_readkey()) != 0) {
		snake_randomize(game, rdtsc());
		if ((next_char == 'w') && (game->snek.direction != DOWN)) {
			game->snek.next_direction = UP;
		} else if ((next_char == 'a') && (game->snek.direction != RIGHT)) {
			game->snek.next_direction = LEFT;
		} else if ((next_char == 's') && (game->snek.direction != UP)) {
			game->snek.next_direction = DOWN;
		} else if ((next_char == 'd') && (game->snek.direction != LEFT)) {
			game->snek.next_direction = RIGHT;
		}
	}
}


bool snake_game_play(struct snake_game_t *game) {
	bool game_running = true;
	const unsigned int fps = 25;
	if (!timer_set(fps)) {
		return false;
	}

	{
		struct cursor_t cursor = {
			.x = 100,
			.y = 30,
		};
		font_printf(&font_vcr_osd_mono_20, &cursor, 0xffffff, 0, "Press ENTER to start game!");
	}
	kbd_waitkey('\r');
	snake_game_print_score(game);

	while (game_running) {
		game_running = snake_game_tick(game);
		timer_wait();
		snake_read_keyboard(game);
	}

	timer_disable();

	{
		snake_clear_status_bar();
		struct cursor_t cursor = {
			.x = 100,
			.y = 30,
		};
		font_printf(&font_vcr_osd_mono_20, &cursor, 0xffffff, 0, "Ooooops you're dead. Final score: %d points! Play again (y/n)?", game->score);
	}
	return kbd_yesno();
}

#ifdef TEST
// gcc -DTEST -no-pie -Wall -std=c11 -fsanitize=address snake_game.c -o test_snek && ./test_snek

#include <stdio.h>

void gfx_draw_pixel(unsigned int x, unsigned int y, uint32_t pixel) {
}

uint32_t kbd_readkey(void) {
	return 0;
}

void snek_pos_dump(struct snake_game_t *game) {
	printf("Snek length %d (should be %d): ", game->snek.shape.length, game->snek.length);
	for (unsigned int i = 0; i < game->snek.shape.length; i++) {
		struct vec2_t *pos = snake_shape_ptr(game, i);
		printf("[ %d %d ] ", pos->x, pos->y);
	}
	printf("\n");
}

int main() {
	struct snake_game_t game;
	snake_game_init(&game, 1920, 1080, 0, 0);
	for (int i = 0; i < 200; i++) {
		snake_game_tick(&game);
		snek_pos_dump(&game);
	}

	uint32_t rnd = 0x123;
	printf("%d\n", rnd);

}
#endif
