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

#define ATA_BASE_PORT			0x1f0
#define ATA_CTRL_BASE_PORT		0x3f6
#define ATA_DATA_REG			(ATA_BASE_PORT + 0)
#define ATA_ERROR_REG			(ATA_BASE_PORT + 1)
#define ATA_SECTOR_CNT_REG		(ATA_BASE_PORT + 2)
#define ATA_SECTOR_LOW_REG		(ATA_BASE_PORT + 3)
#define ATA_SECTOR_MID_REG		(ATA_BASE_PORT + 4)
#define ATA_SECTOR_HIGH_REG		(ATA_BASE_PORT + 5)
#define ATA_DRIVE_HEAD_REG		(ATA_BASE_PORT + 6)
#define ATA_STATUS_REG			(ATA_BASE_PORT + 7)
#define ATA_COMMAND_REG			(ATA_BASE_PORT + 7)
#define ATA_CTRL_REG			(ATA_CTRL_BASE_PORT + 0)

#define ATA_STATUS_FLAG_BUSY	(1 << 7)
#define ATA_STATUS_FLAG_RDY		(1 << 6)
#define ATA_STATUS_FLAG_DRQ		(1 << 3)
#define ATA_CTRL_FLAG_SRST		(1 << 2)

struct partition_t {
	uint8_t status;
	uint8_t chs_start[3];
	uint8_t part_type;
	uint8_t chs_end[3];
	uint32_t lba_start;
	uint32_t length_sectors;
} __attribute__ ((packed));

struct mbr_t {
	uint8_t bootloader[440];
	uint32_t disk_signature;
	uint16_t empty;
	struct partition_t partition[4];
	uint16_t mbr_signature;
} __attribute__ ((packed));

_Static_assert(sizeof(struct mbr_t) == 512, "MBR structure not 512 bytes long");

typedef int (*stage2_fnc_t)(void);

static void cursor_newline(void);

static volatile uint16_t *const screen_base = (volatile uint16_t*)0xb8000;
static struct {
	unsigned int x, y;
} cursor;

static uint8_t port_in(unsigned int address) {
	uint8_t value;
	__asm__ __volatile__("inb (%%dx), %%al" : "=a"(value) : "d"(address));
	return value;
}

static uint16_t port_in_word(unsigned int address) {
	uint16_t value;
	__asm__ __volatile__("inw (%%dx), %%ax" : "=a"(value) : "d"(address));
	return value;
}

static void port_out(unsigned int address, uint8_t value) {
	__asm__ __volatile__("outb %%al, %%dx" : :  "d"(address), "a"(value));
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

static void print_uint32(uint32_t integer) {
	print_byte(integer >> 24);
	print_byte(integer >> 16);
	print_byte(integer >> 8);
	print_byte(integer >> 0);
}

static void print_uint64(uint64_t integer) {
	print_uint32(integer >> 32);
	print_uint32(integer >> 0);
}

#if 0
static void print_hexdump(const uint8_t *data, unsigned int length) {
	const unsigned int line_length = 16;
	for (int i = 0; i < length; i++) {
		print_byte(data[i]);
		print_char(7, ' ');
		if (i && ((i % line_length) == line_length - 1)) {
			cursor_newline();
		}
	}
}
#endif

static void ata_short_delay(void) {
	for (volatile unsigned int i = 0; i < 1000; i++);
}

static void ata_reset(void) {
	port_out(ATA_CTRL_REG, ATA_CTRL_FLAG_SRST);
	ata_short_delay();
	port_out(ATA_CTRL_REG, 0);
	while (true) {
		uint8_t status = port_in(ATA_STATUS_REG);
		if (((status & ATA_STATUS_FLAG_BUSY) == 0) && ((status & ATA_STATUS_FLAG_RDY) != 0)) {
			break;
		}
	}
}

void ata_read_sector(uint32_t lba, void *target) {
	port_out(ATA_DRIVE_HEAD_REG, 0xe0 | ((lba >> 24) & 0x0f));	// LBA, drive 0
	port_out(ATA_SECTOR_CNT_REG, 1);		// 1 sector
	port_out(ATA_SECTOR_LOW_REG, (lba >> 0) & 0xff);
	port_out(ATA_SECTOR_MID_REG, (lba >> 8) & 0xff);
	port_out(ATA_SECTOR_HIGH_REG, (lba >> 16) & 0xff);

	port_out(ATA_COMMAND_REG, 0x20);		// Read with retry
	while ((port_in(ATA_STATUS_REG) & ATA_STATUS_FLAG_BUSY) != 0);	// wait until Busy clear
	while ((port_in(ATA_STATUS_REG) & ATA_STATUS_FLAG_DRQ) == 0);	// wait until DRQ set

	for (int i = 0; i < 512; i += 2) {
		uint16_t data_word = port_in_word(ATA_DATA_REG);
		((uint8_t*)target)[i + 0] = data_word >> 0;
		((uint8_t*)target)[i + 1] = data_word >> 8;
	}
}

static void ata_read_sectors(uint32_t start_lba, uint32_t length_sectors, void *target) {
	for (unsigned int i = 0; i < length_sectors; i++) {
		ata_read_sector(start_lba + i, target + (512 * i));
	}
}

int main64() {
	void *stage2_target_address = (void*)0x40000000;
	cursor.y = 3;
	printmsg("stage1: 64 bit mode successfully entered.\n");

	printmsg("stage1: attempting load of stage2 from partition 2 to ");
	print_uint64((uint64_t)stage2_target_address);
	printmsg("\n");

	ata_reset();
	/* Read MBR first */
	struct mbr_t mbr;
	ata_read_sector(0, &mbr);

	/* Is there a partition 2 entry present? */
	if (mbr.partition[1].length_sectors == 0) {
		printmsg("stage1: unable to find a stage 2 partition (length 0)\n");
	} else {
		printmsg("stage1: found stage 2 at LBA ");
		print_uint32(mbr.partition[1].lba_start);
		printmsg(" length ");
		print_uint32(mbr.partition[1].length_sectors);
		printmsg("\n");
		ata_read_sectors(mbr.partition[1].lba_start, mbr.partition[1].length_sectors, stage2_target_address);

		/* Cast stage2 IVT to function pointer */
		stage2_fnc_t *stage2_ivt = (stage2_fnc_t*)stage2_target_address;
		printmsg("stage1: loaded stage 2, IVT entry 0 points to ");
		print_uint64((uint64_t)stage2_ivt[0]);
		printmsg("\n");

		/* Launch stage 2 */
		stage2_fnc_t stage2_entry = stage2_ivt[0];
		stage2_entry();
	}
	return 0;
}
