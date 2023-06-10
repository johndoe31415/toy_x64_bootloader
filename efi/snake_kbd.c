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
#include <stdbool.h>
#include "snake_kbd.h"

static EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *protocol;


bool kbd_init(void) {
	{
		EFI_STATUS status = uefi_call_wrapper(BS->LocateProtocol, 3, &((EFI_GUID)EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID), NULL, (void**)&protocol);
		if (EFI_ERROR(status)) {
			Print(L"LocateProtocol failed.\n");
			return false;
		}
	}
	return true;
}

uint32_t kbd_readkey(void) {
	EFI_KEY_DATA key_data;
	EFI_STATUS status = uefi_call_wrapper(protocol->ReadKeyStrokeEx, 2, protocol, &key_data);
	if (status == EFI_NOT_READY) {
		/* No key to read */
		return 0;
	} else {
		return key_data.Key.UnicodeChar;
	}
}

void kbd_waitkey(uint32_t key) {
	while (kbd_readkey() != key);
}

bool kbd_yesno(void) {
	uint32_t key;
	while (true) {
		key = kbd_readkey();
		if ((key == 'y') || (key == 'Y')) {
			return true;
		} else if ((key == 'n') || (key == 'N')) {
			return false;
		}
	}
}
