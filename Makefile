TARGET := test.kpm
SRC := main.c
SDK := kp_sdk/include
CC := aarch64-none-elf-gcc
LD := aarch64-none-elf-ld

CFLAGS := -ffreestanding -fno-stack-protector -fno-stack-check
CFLAGS += -fno-PIC -mgeneral-regs-only
CFLAGS += -I$(SDK) -O2 -Wall
LDFLAGS := -r -nostdlib

OBJS := $(SRC:.c=.o)

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) $(TARGET)
