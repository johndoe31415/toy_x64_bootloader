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
#include "snake_timer.h"

static EFI_EVENT timer_event;

bool timer_set(const unsigned int frequency_hz) {
	const uint64_t delay = (uint64_t)10000000 / frequency_hz;
	{
		EFI_STATUS status = uefi_call_wrapper(BS->CreateEvent, 5, EVT_TIMER, TPL_NOTIFY, NULL, NULL, &timer_event);
		if (EFI_ERROR(status)) {
			Print(L"Unable to CreateEvent.\n");
			return false;
		}
	}
	{
		EFI_STATUS status = uefi_call_wrapper(BS->SetTimer, 3, timer_event, TimerPeriodic, delay);
		if (EFI_ERROR(status)) {
			Print(L"Unable to SetTimer.\n");
			return false;
		}
	}
	return true;
}

void timer_wait(void) {
	UINTN index;
	uefi_call_wrapper(BS->WaitForEvent, 3, 1, &timer_event, &index);
}

void timer_disable(void) {
	uefi_call_wrapper(BS->SetTimer, 3, &timer_event, TimerCancel, 0);
	uefi_call_wrapper(BS->CloseEvent, 1, timer_event);
}
