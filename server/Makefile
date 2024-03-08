GCC            := "/usr/bin/gcc"
CROSS_COMPILE  := "aarch64-none-linux-gnu-"
SOURCE := aesdsocket.c
TARGET := aesdsocket

default: 
	$(GCC) $(SOURCE) -o $(TARGET)

CROSS_COMPILE: 
	${CROSS_COMPILE}gcc $(SOURCE) -o $(TARGET)

clean: 
	rm -rf *.o *.elf *.bin *.s $(TARGET)
