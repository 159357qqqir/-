# Makefile
CC = gcc
AS = nasm
LD = ld

# 输出内核镜像
kernel.bin: boot.o kernel.o
	$(LD) -m elf_i386 -o kernel.bin boot.o kernel.o

boot.o: boot.asm
	$(AS) -f elf32 boot.asm -o boot.o

kernel.o: kernel.c
	$(CC) -m32 -ffreestanding -c kernel.c -o kernel.o

clean:
	rm -f *.o kernel.bin
