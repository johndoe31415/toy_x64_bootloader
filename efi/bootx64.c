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
#include <stdint.h>
#include <stdbool.h>

static uint64_t get_cr0(void) {
	uint64_t cr0;
	__asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
	return cr0;
}

static uint64_t* get_cr3(void) {
	uint64_t *cr3;
	__asm__ __volatile__("mov %%cr3, %0" : "=r"(cr3));
	return cr3;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_tbl) {
	InitializeLib(handle, system_tbl);
	Print(L"EFI initialized, efi_main() at 0x%lhx\n", (uint64_t)efi_main);

	uint64_t cr0 = get_cr0();
	uint64_t *cr3 = get_cr3();
	Print(L"CR0 is 0x%lhx, CR3 at 0x%lhx\n", cr0, (uint64_t)cr3);

	for (int i = 0; i < (1 << 9); i++) {
		if (cr3[i] & 1) {
			/* Present */
			Print(L"CR3[%d] entry 0x%lhx\n", i, cr3[i]);
		}
	}

	Print(L"Press any key to terminate EFI application...");
	Pause();
	return EFI_SUCCESS;
}
