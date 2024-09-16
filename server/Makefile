GCC := gcc
CROSS_COMPILE := aarch64-none-linux-gnu-
SOURCE := aesdsocket.c
TARGET := aesdsocket

PREFIX ?= /usr/local
DESTDIR ?= 
INSTALLDIR := $(DESTDIR)$(PREFIX)/bin

default:
	$(GCC) $(SOURCE) -o $(TARGET)

# Cross compile
cross_compile:
	$(CROSS_COMPILE)gcc $(SOURCE) -o $(TARGET)

# Clean
clean:
	rm -rf *.o *.elf *.bin *.s $(TARGET)

# I need to change this for Yocto
install:
	install -d $(INSTALLDIR)
	install -m 0755 $(TARGET) $(INSTALLDIR)
