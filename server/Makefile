# I need to update some of these to be optional, I need to create an all stage, add ld and cc flags ... yocto wants a lot.
CC ?= aarch64-none-linux-gnu-
GCC ?= gcc
SOURCE ?= aesdsocket.c
TARGET ?= aesdsocket
CFLAGS ?= -pthread
LDFLAGS ?=

CPPFLAGS ?=

PREFIX ?= /usr/local
DESTDIR ?= 
INSTALLDIR := $(DESTDIR)$(PREFIX)/bin

# Update to let me choose a new path for the file location for assignment 8+
FILEPATH ?= /var/tmp/aesdsocketdata

# Need to default this for the assignment 8
USE_AESD_CHAR_DEVICE ?= 1

ifeq ($(USE_AESD_CHAR_DEVICE), 1)
    FILEPATH := /dev/aesdchar
	CPPFLAGS += -DUSE_AESD_CHAR_DEVICE
endif

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
	rm -rf *.o *.elf *.bin *.s aesdsocket $(TARGET)

# I need to change this for Yocto
install:
	install -d $(INSTALLDIR)
	install -m 0755 $(TARGET) $(INSTALLDIR)
