OBJ=bootstub.o head.o

all: bootstub.bin

bootstub.bin:bootstub.elf
	objcopy -O binary -R .note -R .comment -S $< $@

bootstub.elf:bootstub.lds $(OBJ)
	ld -m elf_i386 -T bootstub.lds $(OBJ) -o $@

bootstub.o:bootstub.c
	gcc -c bootstub.c

head.o:head.S
	gcc -c head.S

clean:
	rm -rf *.o *.bin *.elf

.PHONY: all clean
