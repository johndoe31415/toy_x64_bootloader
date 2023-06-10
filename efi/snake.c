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

#include <efi.h>
#include <efilib.h>
#include <efibind.h>
#include <stdint.h>
#include <stdbool.h>
#include "snake_gfx.h"
#include "snake_kbd.h"
#include "snake_game.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_tbl) {
	InitializeLib(handle, system_tbl);

	if (!gfx_init()) {
		Print(L"GFX initialization failed, sad :(\n");
		Pause();
		return EFI_UNSUPPORTED;
	}

	if (!kbd_init()) {
		Print(L"Keyboard initialization failed, sad :(\n");
		Pause();
		return EFI_UNSUPPORTED;
	}

	unsigned int screen_width, screen_height;
	gfx_get_resolution(&screen_width, &screen_height);

	while (true) {
		struct snake_game_t game;
		snake_game_init(&game, screen_width - 100, screen_height - 100, 50, 50);
		bool play_again = snake_game_play(&game);
		if (!play_again) {
			break;
		}
	}

	return EFI_SUCCESS;
}
