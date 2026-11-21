# 交叉编译器前缀，可通过环境变量覆盖
CROSS_COMPILE ?= aarch64-none-elf-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

# KernelPatch 源码路径，CI自动克隆，本地可自行指定
KP_DIR ?= KernelPatch
INC := -I$(KP_DIR)/include -I$(KP_DIR)/arch/arm64/include

# 编译选项
CFLAGS := -O2 -Wall -fno-builtin -nostdinc -nostdlib -fno-common \
          -fno-pie -no-pie -mgeneral-regs-only -ffreestanding \
          -DCONFIG_ARM64 -D__KERNEL__ $(INC)

LDFLAGS := -r -EL -maarch64elf

# 目标文件
TARGET := kpm-hello-demo.kpm
OBJ := main.o

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -f *.o $(TARGET)

.PHONY: all clean
