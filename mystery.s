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

.code16
.text

.globl main
main:
	# Set VGA video mode, 80x25
	mov $0x03, %ax
	mov $0, %bx
	int $0x10

	# ES is screen segment
	mov $0xb800, %ax
	mov %ax, %es

	# DS is zero
	xor %ax, %ax
	mov %ax, %ds

	# Decode source string by XORing with PRNG from LFSR
	mov $hello_string, %si
	call decode_string

	# Initialize cursor
	mov $0, %di
	print_loop:
		mov $hello_string, %si	# Source string
		call printstr
	jmp  print_loop
hlt


next_color:
	# Increase color index
	inc %dl

	# Check if foreground color...
	mov %dl, %bl
	and $0xf, %bl

	# ...is equal to background color
	mov %dl, %cl
	shr $4, %cl
	and $0xf, %cl

	# If so, increase again. Else finished.
	test %bl, %cl
	je next_color
ret


# Print character %al to cursor %di
# Color attribute at %dl
printchar:
	mov %al, %es:(%di)
	mov %dl, %es:1(%di)
ret

# Print string at ds:si to screen at current cursor es:di
printstr:
	# Is the string finished (zero-terminated)?
	movb %ds:(%si), %al
	test %al, %al
	jz printstr_finished

	# No, we have a character to print in %al
	call next_color
	call printchar
	call delay_fnc
	inc %si
	add $2, %di

	# Check if we're at end of screen
	cmp $(80*25*2), %di
	jnz printstr

	# Yes, we are, go back to cursor position 0
	mov $0, %di
	jmp printstr
printstr_finished:
ret


delay_fnc:
	mov $0x800000, %ecx
	continue_delay:
		dec %ecx
		test %ecx, %ecx
	jnz continue_delay
ret


# XOR string in ds:si against PRNG pattern until string zero-terminates
decode_string:
	mov $55557, %cx

	decode_string_next:
		shr $1, %cx
		jnc decode_string_skip_xor
			xor $0x1021, %cx
		decode_string_skip_xor:

		# Load character
		movb %ds:(%si), %al

		# XOR with next state of PRNG
		xor %cl, %al

		# Write character back
		movb %al, %ds:(%si)

		# Increase pointer
		inc %si

		# Is the string zero-terminated?
		test %al, %al
	jnz decode_string_next
ret


.section .data
	hello_string:
	.byte 225, 21, 76, 238, 167, 130, 48, 218, 6, 98, 95, 226, 46, 233, 167, 193, 99, 56, 230, 32, 41, 173, 230, 195, 240, 200, 116
