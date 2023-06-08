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

#include <stdint.h>
#include <stdbool.h>

static void cursor_newline(void);
static void printmsg(const char *message);
static void print_byte(uint8_t byte);

static volatile uint16_t *const screen_base = (volatile uint16_t*)0xb8000;
static struct {
	unsigned int x, y;
} cursor;

static uint8_t port_in(unsigned int address) {
	uint8_t value;
	__asm__ __volatile__("inb (%%dx), %%al" : "=a"(value) : "d"(address));
	return value;
}

static bool is_key_pressed(void){
	return port_in(0x64) & (1 << 0);
}

static void wait_until_key_pressed(void) {
	while (!is_key_pressed());
}

static uint8_t read_pressed_key(void) {
	wait_until_key_pressed();
	return port_in(0x60);
}

static void monitor_keypresses(void) {
	while (true) {
		uint8_t key = read_pressed_key();
		printmsg("stage2: keypress ");
		print_byte(key);
		printmsg(" (");
		if (key & 0x80) {
			printmsg("up");
		} else {
			printmsg("down");
		}
		printmsg(" ");
		print_byte(key & ~0x80);
		printmsg(")\n");
	}
}

static void print_char_at(int x, int y, uint8_t color, uint8_t character) {
	volatile uint16_t *screen_pos = screen_base + (80 * y) + x;
	*screen_pos = (color << 8) | character;
}

static void print_char(uint8_t color, uint8_t character) {
	print_char_at(cursor.x, cursor.y, color, character);
	cursor.x = (cursor.x + 1) % 80;
	if (cursor.x == 0) {
		cursor_newline();
	}
}

static void fillscr(uint8_t color, char fillchar) {
	for (int y = 0; y < 25; y++) {
		for (int x = 0; x < 80; x++) {
			print_char_at(x, y, color, fillchar);
		}
	}
	cursor.x = 0;
	cursor.y = 0;
}

static void clrscr(void) {
	fillscr(7, ' ');
}

static void cursor_newline(void) {
	cursor.x = 0;
	cursor.y = (cursor.y + 1) % 25;
	if (cursor.y == 0) {
		clrscr();
	}
}

static void printmsg(const char *message) {
	while (*message) {
		if (*message == '\n') {
			cursor_newline();
		} else {
			print_char(0x07, *message);
		}
		message++;
	}
}

static void print_nibble(uint8_t nibble) {
	nibble &= 0x0f;
	if (nibble < 10) {
		print_char(0x07, '0' + nibble);
	} else {
		print_char(0x07, 'a' + nibble - 10);
	}
}

static void print_byte(uint8_t byte) {
	print_nibble(byte >> 4);
	print_nibble(byte >> 0);
}

static void print_int32(uint32_t integer) {
	print_byte(integer >> 24);
	print_byte(integer >> 16);
	print_byte(integer >> 8);
	print_byte(integer >> 0);
}

static void print_uint64(uint64_t integer) {
	print_int32(integer >> 32);
	print_int32(integer >> 0);
}

int stage2_main(void) {
	cursor.y = 8;

	printmsg("stage2: successfully initialized. Application now running.\n");

	printmsg("stage2: address of stage2_main(): 0x");
	print_uint64((uint64_t)stage2_main);
	printmsg("\n");

	monitor_keypresses();
	return 0;
}

/* Vector table only has one entry, which points to main() */
int (*stage2_ivt[])(void) __attribute__ ((section (".ivt"))) = {
	stage2_main,
};
