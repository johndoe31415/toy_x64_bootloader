.PHONY: all clean test debug
.SUFFIXES: .so .efi

TARGETS := bootx64.efi uefisnek.efi
TARGET1_OBJS := bootx64.o
TARGET2_OBJS := snake.o snake_gfx.o snake_kbd.o snake_font.o vcr-osd-mono-20.o snake_timer.o snake_game.o

LDSCRIPT := /usr/lib/elf_x86_64_efi.lds
CFLAGS := -O3 -Wall -ggdb3 -std=c11 -I/usr/include/efi -I/usr/include/efi/x86_64 -pie -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPPER
LDFLAGS := -shared -nostdlib -znocombreloc -T$(LDSCRIPT) -Bsymbolic -L/usr/lib /usr/lib/crt0-efi-x86_64.o

all: $(TARGETS)

bootx64.so: $(TARGET1_OBJS)
	ld $(LDFLAGS) -o $@ $^ -lefi -lgnuefi

uefisnek.so: $(TARGET2_OBJS)
	ld $(LDFLAGS) -o $@ $^ -lefi -lgnuefi

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $^

.so.efi:
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .reloc --target=efi-app-x86_64 $^ $@

clean:
	rm -f $(TARGETS)
	rm -f bootx64.so
	rm -f $(TARGET1_OBJS)

test: $(TARGETS)
	@mkdir -p root/efi/boot/
	cp uefisnek.efi root/efi/boot/bootx64.efi
	qemu-system-x86_64 -drive if=pflash,format=raw,file=OVMF.fd -drive format=raw,file=fat:rw:root -net none

debug: $(TARGETS)
	@mkdir -p root/efi/boot/
	cp uefisnek.efi root/efi/boot/bootx64.efi
	#qemu-system-x86_64 -drive if=pflash,format=raw,file=OVMF.fd -drive format=raw,file=fat:rw:root -net none -serial stdio -gdb tcp::9000 -S
	qemu-system-x86_64 -drive if=pflash,format=raw,file=OVMF.fd -drive format=raw,file=fat:rw:root -net none -serial stdio
