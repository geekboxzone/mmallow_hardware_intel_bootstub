all: bootstub.bin

bootstub.bin:bootstub.elf
	objcopy -O binary -R .note -R .comment -S bootstub.elf bootstub.bin

bootstub.elf:bootstub.o head.o bootstub.lds
	ld -m elf_i386 -T bootstub.lds head.o bootstub.o -o bootstub.elf

bootstub.o:bootstub.c
	gcc -c bootstub.c

head.o:head.S
	gcc -c head.S

clean:
	rm -rf *.o *.bin *.elf
