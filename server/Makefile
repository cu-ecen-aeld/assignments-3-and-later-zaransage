# I need to update some of these to be optional, I need to create an all stage, add ld and cc flags ... yocto wants a lot.
GCC := gcc
CROSS_COMPILE := aarch64-none-linux-gnu-
SOURCE := aesdsocket.c
TARGET := aesdsocket
CFLAGS := -pthread

PREFIX ?= /usr/local
DESTDIR ?= 
INSTALLDIR := $(DESTDIR)$(PREFIX)/bin

default:
	$(GCC) $(SOURCE) -o $(TARGET) $(CFLAGS)

# Cross compile
cross_compile:
	$(CROSS_COMPILE)gcc $(SOURCE) -o $(TARGET) $(CFLAGS)

# Clean
clean:
	rm -rf *.o *.elf *.bin *.s $(TARGET)

# I need to change this for Yocto
install:
	install -d $(INSTALLDIR)
	install -m 0755 $(TARGET) $(INSTALLDIR)
