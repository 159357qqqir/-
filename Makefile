NDK_PATH ?= /opt/android-ndk
TOOLCHAIN := $(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64
CC := $(TOOLCHAIN)/bin/aarch64-linux-android34-clang
CFLAGS := -O3 -Wall -pthread
LDFLAGS := -lEGL -lGLESv2

all: nb

nb: nb.c
	$(CC) $(CFLAGS) nb.c -o nb $(LDFLAGS)

clean:
	rm -f nb
