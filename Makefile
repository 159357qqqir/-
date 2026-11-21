CC := aarch64-none-elf-gcc
LD := aarch64-none-elf-ld
OBJCOPY := aarch64-none-elf-objcopy
KP_DIR := ./KernelPatch

CFLAGS := -ffreestanding -fno-stack-protector -fno-PIC -mno-outline-atomics -std=c11 -I$(KP_DIR)/include
LDFLAGS := -r -T $(KP_DIR)/scripts/kpm.ld

demo.kpm: demo.c
	$(CC) $(CFLAGS) -c demo.c -o demo.o
	$(LD) demo.o $(LDFLAGS) -o demo.kpm

clean:
	rm -f demo.o demo.kpm

