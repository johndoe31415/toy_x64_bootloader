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

#include <stdio.h>
#include <stdarg.h>
#include "snake_font.h"
#include "snake_gfx.h"

void font_blit_glyph(const struct glyph_t *glyph, const int x0, const int y0, uint32_t color_on, uint32_t color_off) {
	const int glyph_rowwidth = (glyph->width + 7) / 8;
	for (int y = 0; y < glyph->height; y++) {
		for (int x = 0; x < glyph->width; x++) {
			const int byte_offset = (x / 8) + (y * glyph_rowwidth);
			const int bit_offset = x % 8;
			const bool pixel = (((glyph->data[byte_offset] >> bit_offset) & 1) != 0);
			if (pixel) {
				gfx_draw_pixel(x0 + x + glyph->xoffset, y0 + y + glyph->yoffset, color_on);
			} else {
				gfx_draw_pixel(x0 + x + glyph->xoffset, y0 + y + glyph->yoffset, color_off);
			}
		}
	}
}

void font_write(const struct font_t *font, struct cursor_t *cursor, const char *text, uint32_t color_on, uint32_t color_off) {
	while (*text) {
		char next_char = *text;
		int index = font->codepoint_to_charindex_fn(next_char);
		if (index != -1) {
			const struct glyph_t *glyph = &font->glyphs[index];
			font_blit_glyph(glyph, cursor->x, cursor->y, color_on, color_off);
			cursor->x += glyph->xadvance;
		}
		text++;
	}
}

void font_printf(const struct font_t *font, struct cursor_t *cursor, uint32_t color_on, uint32_t color_off, const char *msg, ...) {
	char text[256];
	va_list ap;
	va_start(ap, msg);
	AsciiVSPrint(text, sizeof(text), msg, ap);
	va_end(ap);

	font_write(font, cursor, text, color_on, color_off);
}
