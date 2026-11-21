obj-m += selinux_perm.o
KDIR ?= $(KERNEL_SRC)
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
