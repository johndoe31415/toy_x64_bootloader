/**
 *	flightpanel - A Cortex-M4 based USB flight panel for flight simulators.
 *	Copyright (C) 2017-2017 Johannes Bauer
 *
 *	This file is part of flightpanel.
 *
 *	flightpanel is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; this program is ONLY licensed under
 *	version 3 of the License, later versions are explicitly excluded.
 *
 *	flightpanel is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with flightpanel; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	Johannes Bauer <JohannesBauer@gmx.de>
**/

#ifndef __SNAKE_FONT_H__
#define __SNAKE_FONT_H__

#include <stdint.h>
#include <stdbool.h>

typedef int (*cp_to_charindex_t)(const unsigned int codepoint);

struct glyph_t {
	uint8_t xadvance;
	int8_t xoffset, yoffset;
	uint8_t width, height;
	const uint8_t *data;
};

struct font_t {
	cp_to_charindex_t codepoint_to_charindex_fn;
	struct glyph_t glyphs[];
};

struct cursor_t {
	int x, y;
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
void font_blit_glyph(const struct glyph_t *glyph, const int x0, const int y0, uint32_t color_on, uint32_t color_off);
void font_write(const struct font_t *font, struct cursor_t *cursor, const char *text, uint32_t color_on, uint32_t color_off);
void font_printf(const struct font_t *font, struct cursor_t *cursor, uint32_t color_on, uint32_t color_off, const char *msg, ...);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
