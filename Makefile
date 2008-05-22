OBJ=bootstub.o head.o

all: bootstub

bootstub:bootstub.bin
	cat bootstub.bin /dev/zero | dd bs=4096 count=1 > $@

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

source:bootstub.c head.S VERSION
	git-archive --prefix=bootstub-`head -n 1 VERSION | awk '{print $$1}'`/ --format=tar HEAD | bzip2 -c > bootstub-`head -n 1 VERSION | awk '{print $$1}'`.tar.bz2

rpm:source
	cp bootstub.spec /usr/src/redhat/SPECS/
	cp *.tar.bz2 /usr/src/redhat/SOURCES/
	rpmbuild -ba /usr/src/redhat/SPECS/bootstub.spec
	cp /usr/src/redhat/RPMS/i386/bootstub*.rpm ./
.PHONY: all clean source rpm
