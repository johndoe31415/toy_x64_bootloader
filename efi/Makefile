.PHONY: all clean test

TARGET := bootx64.efi
OBJS := bootx64.o

LDSCRIPT := /usr/lib/elf_x86_64_efi.lds
CFLAGS := -O2 -Wall -I/usr/include/efi -pie -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPPER
LDFLAGS := -shared -nostdlib -znocombreloc -T$(LDSCRIPT) -Bsymbolic -L/usr/lib /usr/lib/crt0-efi-x86_64.o

all: $(TARGET)

bootx64.efi: bootx64.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .reloc --target=efi-app-x86_64 $^ $@

bootx64.so: $(OBJS)
	ld $(LDFLAGS) -o $@ $^ -lefi -lgnuefi

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS)

test: $(TARGET)
	@mkdir -p root/efi/boot/
	cp $(TARGET) root/efi/boot/bootx64.efi
	qemu-system-x86_64 -drive if=pflash,format=raw,file=OVMF.fd -drive format=raw,file=fat:rw:root -net none
