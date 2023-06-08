#	toy_x64_bootloader - Minimal bootloader for x86_64 using long mode and PML4
#	Copyright (C) 2023-2023 Johannes Bauer
#
#	This file is part of toy_x64_bootloader.
#
#	toy_x64_bootloader is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; this program is ONLY licensed under
#	version 3 of the License, later versions are explicitly excluded.
#
#	toy_x64_bootloader is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with toy_x64_bootloader; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#	Johannes Bauer <JohannesBauer@gmx.de>

# Vol 3A 5-3 5.2 Fields and flags used for segment-level and page-level protection, pg. 3158
.equ SD_L,					(1 << 21)		# 64 bit segment
.equ SD_AVL,				(1 << 20)		# Available
.equ SD_DB,					(1 << 22)		# Default operation size (set = 32-bit segment)
.equ SD_DPL_0,				(0 << 13)		# Descriptor privilege level
.equ SD_DPL_1,				(1 << 13)
.equ SD_DPL_2,				(2 << 13)
.equ SD_DPL_3,				(3 << 13)
.equ SD_G,					(1 << 23)		# Granularity (0 = 1 byte, 1 = 4 kiB)
.equ SD_P,					(1 << 15)		# Segment present
.equ SD_S,					(1 << 12)		# Descriptor type (0 = system, 1 = code/data)

.equ SD_TYPE_DS,			(0 << 11)		# Segment type for data segment
.equ SD_TYPE_DS_A,			(1 << 8)		# Segment type for data segment: accessed
.equ SD_TYPE_DS_W,			(1 << 9)		# Segment type for data segment: writable
.equ SD_TYPE_DS_E,			(1 << 10)		# Segment type for data segment: expansion direction

.equ SD_TYPE_CS,			(1 << 11)		# Segment type for code segment
.equ SD_TYPE_CS_A,			(1 << 8)		# Segment type for code segment: accessed
.equ SD_TYPE_CS_R,			(1 << 9)		# Segment type for code segment: readable
.equ SD_TYPE_CS_C,			(1 << 10)		# Segment type for code segment: conforming

.equ SD_SEGTYPE_CODE_RX,	(SD_S | SD_TYPE_CS | SD_TYPE_CS_R)
.equ SD_SEGTYPE_DATA_RW,	(SD_S | SD_TYPE_DS | SD_TYPE_DS_W)

.macro segment_descriptor base, limit, flags
	.long (\limit & 0xffff) | ((\base & 0xffff) << 16)
	.long (\flags) | (\base & 0xff000000) | ((\base & 0x00ff0000) >> 16) | (\limit & 0xf0000)
.endm


.code16
.text

.globl main
main:
	# Initialize stack pointer
	xor %ax, %ax
	mov %ax, %ss
	mov $0x7fff, %sp

	# Set VGA video mode, 80x25 (clears screen)
	mov $0x03, %ax
	mov $0, %bx
	int $0x10

	# Show first message
	mov $str_stage0_init, %si
	call print_string

	# Target memory 0x8000 = 0000:8000 = 0800:0000
	mov $0x800, %ax
	mov %ax, %es
	mov $0x0, %bx	

	# Load 128 sectors to 0x8000
	mov $2, %ah			# Read
	mov $128, %al		# number of sectors to read
	mov $0, %ch			# cylinder
	mov $2, %cl			# sector
	mov $0, %dh			# head
	mov $0x80, %dl		# first hard drive
	int $0x13

	jmp switch_to_protected_mode

switch_to_protected_mode:
	# Disable IRQs
	cli

	# Load GDT
	lgdt (gdt_desc)

	# Last message in real mode (afterwards we do not have BIOS INTs)
	mov $str_stage0_switch_protected_mode, %si
	call print_string

	# Prepare switch to protected mode
	mov $1, %ax
	lmsw %ax

	# Far jump initiates actual jump to protected mode
	jmp $8, $init_32bit

print_string:
	mov $0x0e, %ah
	lodsb
	test %al, %al
	jz print_string_finished
		int $0x10
	jmp print_string

print_string_finished:
    ret

str_stage0_init:
	.string "stage0: video mode initialized\r\n"

str_stage0_switch_protected_mode:
	.string "stage0: will now switch into protected mode\r\n"

.code32
init_32bit:
	# cs has been initialized by far jump
	# initilize remaining segment descriptors
	mov $16, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %gs
	mov %ax, %ss

	# set a new stack pointer at 2 MiB
	mov $0x200000, %esp
	
	# load the entry point at 0x8000 from the IVT
	movl 0x8000, %eax

	# start the main program in 32 bit mode
	jmp *%eax

.section .data
gdt:
	gdt_entry_null:	segment_descriptor 0, 0, 0
	gdt_entry_cs: 	segment_descriptor 0, 0xfffff, SD_SEGTYPE_CODE_RX | SD_P | SD_DB | SD_G
	gdt_entry_ds: 	segment_descriptor 0, 0xfffff, SD_SEGTYPE_DATA_RW | SD_P | SD_DB | SD_G
gdt_end:

gdt_desc:
	.word gdt_end - gdt - 1		# size of GDT
	.long gdt					# offset of GDT
