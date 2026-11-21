TARGET := test.kpm
SRC := src/main.c
KP_DIR := kp
TARGET_COMPILE ?= aarch64-none-elf-
CC := $(TARGET_COMPILE)gcc
LD := $(TARGET_COMPILE)ld
OBJCOPY := $(TARGET_COMPILE)objcopy

CFLAGS := -ffreestanding -fno-stack-protector -fno-stack-check
CFLAGS += -fno-PIC -mno-red-zone -mgeneral-regs-only
CFLAGS += -I$(KP_DIR)/include -O2 -Wall
LDFLAGS := -r -nostdlib

OBJS := $(SRC:.c=.o)
all: $(TARGET)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@
clean:
	rm -f $(OBJS) $(TARGET)
