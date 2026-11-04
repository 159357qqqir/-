CC = gcc
AS = nasm
LD = ld

# 输出elf内核
kernel.elf: boot.o kernel.o
	$(LD) -m elf_i386 -o kernel.elf boot.o kernel.o

boot.o: boot.asm
	$(AS) -f elf32 boot.asm -o boot.o

kernel.o: kernel.c
	$(CC) -m32 -ffreestanding -c kernel.c -o kernel.o

# 打包成ISO
os.iso: kernel.elf grub.cfg
	mkdir -p iso_root/boot/grub
	cp kernel.elf iso_root/boot/
	cp grub.cfg iso_root/boot/grub/
	grub-mkrescue -o os.iso iso_root

clean:
	rm -f *.o kernel.elf os.iso
	rm -rf iso_root
