# I need to update some of these to be optional, I need to create an all stage, add ld and cc flags ... yocto wants a lot.
CC ?= aarch64-none-linux-gnu-
GCC ?= gcc
SOURCE ?= aesdsocket.c
TARGET ?= aesdsocket
CFLAGS ?= -pthread
LDFLAGS ?=

PREFIX ?= /usr/local
DESTDIR ?= 
INSTALLDIR := $(DESTDIR)$(PREFIX)/bin

# Might be needed for Yocto
all:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCE) -o $(TARGET) 

default:
	$(GCC) $(CFLAGS) $(LDFLAGS) $(SOURCE) -o $(TARGET) 

# Cross compile
cross_compile:
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCE) -o $(TARGET) 

# Clean
clean:
	rm -rf *.o *.elf *.bin *.s $(TARGET)

# I need to change this for Yocto
install:
	install -d $(INSTALLDIR)
	install -m 0755 $(TARGET) $(INSTALLDIR)
