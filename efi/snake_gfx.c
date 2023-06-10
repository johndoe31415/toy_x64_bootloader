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
#include "snake_gfx.h"

#define ABSDIFF(X, Y)		(((X) > (Y)) ? ((X) - (Y)) : ((Y) - (X)))

static struct gfx_state_t {
	EFI_GRAPHICS_OUTPUT_PROTOCOL *protocol;
	unsigned int screen_width, screen_height;
	unsigned int pixels_per_scanline;
	uint32_t *screen;
} gfx;

void gfx_get_resolution(unsigned int *screen_width, unsigned int *screen_height) {
	*screen_width = gfx.screen_width;
	*screen_height = gfx.screen_height;
}

void gfx_draw_pixel(unsigned int x, unsigned int y, uint32_t pixel) {
	gfx.screen[(y * gfx.pixels_per_scanline) + x] = pixel;
}

static UINTN gfx_getmode(void) {
	if ((gfx.protocol == NULL) || (gfx.protocol->Mode == NULL)) {
		return 0;
	} else {
		return gfx.protocol->Mode->Mode;
	}
}

static UINTN gfx_mode_count(void) {
	return gfx.protocol->Mode->MaxMode;
}

void gfx_test_pattern(void) {
	for (unsigned int y = 0; y < gfx.screen_height; y++) {
		for (unsigned int x = 0; x < gfx.screen_width; x++) {
			gfx_draw_pixel(x, y, x ^ y);
		}
	}
}

void gfx_fill(unsigned int xoffset, unsigned int yoffset, unsigned int width, unsigned int height, uint32_t pixel) {
	for (unsigned int y = yoffset; y < yoffset + height; y++) {
		for (unsigned int x = xoffset; x < xoffset + width; x++) {
			gfx_draw_pixel(x, y, pixel);
		}
	}

}

void gfx_fill_screen(uint32_t pixel) {
	gfx_fill(0, 0, gfx.screen_width, gfx.screen_height, pixel);
}

static UINTN gfx_mode_fitness(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info) {
	UINTN pixel_count = info->HorizontalResolution * info->VerticalResolution;
	return ABSDIFF(pixel_count, 1920 * 1080);
}

bool gfx_init(void) {
	{
		EFI_STATUS status = uefi_call_wrapper(BS->LocateProtocol, 3, &((EFI_GUID)EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID), NULL, (void**)&gfx.protocol);
		if (EFI_ERROR(status)) {
			Print(L"LocateProtocol GRAPHICS_OUTPUT failed.\n");
			return false;
		}
	}
	{
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
		UINTN sizeof_info;
		EFI_STATUS status = uefi_call_wrapper(gfx.protocol->QueryMode, 4, gfx.protocol, gfx_getmode(), &sizeof_info, &info);
		if (EFI_ERROR(status)) {
			Print(L"QueryMode failed.\n");
			return false;
		}
	}

	{
		UINTN best_fitness = (UINTN)-1;
		UINTN best_mode = (UINTN)-1;
		UINTN mode_count = gfx_mode_count();
		for (UINTN i = 0; i < mode_count; i++) {
			EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
			UINTN sizeof_info;
			EFI_STATUS status = uefi_call_wrapper(gfx.protocol->QueryMode, 4, gfx.protocol, i, &sizeof_info, &info);
			if (EFI_ERROR(status)) {
				continue;
			}
			if ((info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor) || (info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor)) {
				/*  RGB mode or BGR mode */
				if (gfx_mode_fitness(info) < best_fitness) {
					best_mode = i;
					best_fitness = gfx_mode_fitness(info);
				}
			}
		}
		if (best_mode == (UINTN)-1) {
			Print(L"No suitable mode found.\n");
			return false;
		}

		{
			EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
			UINTN sizeof_info;
			EFI_STATUS status = uefi_call_wrapper(gfx.protocol->QueryMode, 4, gfx.protocol, best_mode, &sizeof_info, &info);
			if (EFI_ERROR(status)) {
				Print(L"QueryMode(%d) failed.\n", best_mode);
				return false;
			}

			//Print(L"Switching to mode %d: %d x %d (fmt %d)\n", best_mode, info->HorizontalResolution, info->VerticalResolution, info->PixelFormat);

			status = uefi_call_wrapper(gfx.protocol->SetMode, 2, gfx.protocol, best_mode);
			if (EFI_ERROR(status)) {
				Print(L"SetMode(%d)/gfx failed.\n", best_mode);
				return false;
			}

			gfx.screen_width = info->HorizontalResolution;
			gfx.screen_height = info->VerticalResolution;
			gfx.pixels_per_scanline = info->PixelsPerScanLine;
			gfx.screen = (uint32_t*)gfx.protocol->Mode->FrameBufferBase;
		}
	}
	return true;
}
