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

#ifndef __SNAKE_GFX_H__
#define __SNAKE_GFX_H__

#include <stdint.h>
#include <stdbool.h>

#define COLOR_BLACK 0

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
void gfx_get_resolution(unsigned int *screen_width, unsigned int *screen_height);
void gfx_draw_pixel(unsigned int x, unsigned int y, uint32_t pixel);
void gfx_test_pattern(void);
void gfx_fill(unsigned int xoffset, unsigned int yoffset, unsigned int width, unsigned int height, uint32_t pixel);
void gfx_fill_screen(uint32_t pixel);
bool gfx_init(void);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
