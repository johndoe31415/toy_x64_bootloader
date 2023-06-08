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

.equ PG_PRESENT,			(1 << 0)
.equ PG_ALLOW_WRITE,		(1 << 1)
.equ PG_PS,					(1 << 7)

.code32

.globl main
main:

.globl main32
main32:
	# 10.8.5 Initializing IA-32e Mode
	# Right now, PE = 1, PG = 0, PAE = 0, LME = 0

	# Step 2: Enable PAE
	mov %cr4, %eax
	or $0x20, %eax
	mov %eax, %cr4

	# Step 3: Load %cr3 with PML4
	movl $initial_pml4, %eax
	mov %eax, %cr3

	# Step 4: set LME in IA32_EFER MSR
	mov $0xc0000080, %ecx
	rdmsr
	or $0x100, %eax
	wrmsr
	
	# Step 5: Set PG = 1 in %cr0
	mov %cr0, %eax
	or $0x80000000, %eax
	mov %eax, %cr0

	# We are now in compatibility mode.
	# Leave compatibility mode and enter full long mode by setting GDT64
	lgdt (gdt64_desc)
	
	# Far jump makes switch to full long mode effective
	jmp $8, $main32_to_64

.code64
main32_to_64:
	mov $16, %rax
	mov %rax, %ds
	mov %rax, %ss
	mov %rax, %es
	mov %rax, %fs
	mov %rax, %gs

	# Now call the actual high-level program
	call main64
	
	return_from_main64:
	hlt
	jmp return_from_main64		
	

.section .ivt
ivt:
	.long main32

# Identity mapped first 2 MiB of memory
# Map second 2 MiB chunk at linear 0x40000000 (1 GiB) to physical 0x2000000 (32 MiB) for stage 2
gdt64:
	gdt64_entry_null:	segment_descriptor 0, 0, 0
	gdt64_entry_cs: 	segment_descriptor 0, 0xfffff, SD_SEGTYPE_CODE_RX | SD_P | SD_G | SD_L
	gdt64_entry_ds: 	segment_descriptor 0, 0xfffff, SD_SEGTYPE_DATA_RW | SD_P | SD_G | SD_L
gdt64_end:

gdt64_desc:
	.word gdt64_end - gdt64 - 1		# size of GDT
	.long gdt64						# offset of GDT

.align 4096
initial_pml4:
	.quad (PG_PRESENT | PG_ALLOW_WRITE) + initial_pdptr
	.skip 8 * 511

.align 4096
initial_pdptr:
	.quad (PG_PRESENT | PG_ALLOW_WRITE) + initial_pdir
	.quad (PG_PRESENT | PG_ALLOW_WRITE) + stage2_pdir
	.skip 8 * 510

.align 4096
initial_pdir:
	.quad PG_PRESENT | PG_ALLOW_WRITE | PG_PS
	.skip 8 * 511

.align 4096
stage2_pdir:
	.quad PG_PRESENT | PG_ALLOW_WRITE | PG_PS | (32 * 1024 * 1024)
	.skip 8 * 511

.section	.note.GNU-stack,"",@progbits
