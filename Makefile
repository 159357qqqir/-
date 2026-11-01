CC = aarch64-none-elf-gcc
LD = aarch64-none-elf-ld
OBJCOPY = aarch64-none-elf-objcopy

CFLAGS := -ffreestanding
CFLAGS += -fno-stack-protector
CFLAGS += -fno-stack-check
CFLAGS += -fno-PIE
CFLAGS += -mno-red-zone
CFLAGS += -mgeneral-regs-only
CFLAGS += -O2 -Wall -nostdinc

LDFLAGS := -nostdlib -static -T kpm.ld

SRC = src/main.c
OBJ = build/main.o
ELF = build/out.elf
KPM = build/module.kpm

.PHONY: all clean

all: $(KPM)

build:
	mkdir -p build

$(OBJ): $(SRC) | build
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

$(KPM): $(ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf build
