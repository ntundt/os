QEMU_PATH_I386 = /mnt/e/Apps/qemu/qemu-system-i386.exe
DEBUG_DIR = ./debug
DEBUG_FILE = bootloader2.o.debug
TERMINAL_OPTIONS = -drive format=raw,file=./build/floppy.img
tostripfile = ./build/bootloader2.o


build: build/floppy.img

run: build/floppy.img
	$(QEMU_PATH_I386) $(TERMINAL_OPTIONS)

debug: build/floppy.img
	$(QEMU_PATH_I386) $(TERMINAL_OPTIONS) -s -S &
	objcopy --only-keep-debug "${tostripfile}" "${DEBUG_DIR}/${DEBUG_FILE}"
	sleep 5
	gdb -x ./boot.gdb

build/bootloader_stdlib.o: bootloader_stdlib.c
	gcc -m32 -c bootloader_stdlib.c -o ./build/bootloader_stdlib.o -nostdlib -fno-pic -g

build/bootloader-writer/bootloader-write: ./bootloader-writer/bootloader-write.c
	gcc -o build/bootloader-writer/bootloader-write ./bootloader-writer/bootloader-write.c -g

build/boot.bin: boot.asm
	nasm boot.asm -f bin -o ./build/boot.bin

build/boot2.o: boot2.asm
	nasm -f elf32 boot2.asm -o ./build/boot2.o

build/bootloader.o: bootloader.c
	gcc -m32 -c bootloader.c -o ./build/bootloader.o -nostdlib -fno-pic -O0 -g -fno-builtin

build/bootloader2.o: build/bootloader.o build/boot2.o bootload.sys.ld \
	build/screen/screen.o build/cpuio/cpuio.o build/bootloader_stdlib.o \
	build/screen/stdio.o build/screen/panic.o build/interrupts.o \
	build/fs/floppy.o build/ps2/keyboard.o
	ld -m elf_i386 -T bootload.sys.ld -o ./build/bootloader2.o ./build/boot2.o \
		./build/bootloader.o ./build/screen/screen.o ./build/cpuio/cpuio.o \
		./build/bootloader_stdlib.o ./build/screen/stdio.o \
		./build/screen/panic.o ./build/interrupts.o ./build/fs/floppy.o \
		./build/ps2/keyboard.o

build/BOOTLOAD.SYS: build/bootloader2.o
	objcopy -O binary -j .text -j .bss -j .data -j .rodata ./build/bootloader2.o ./build/BOOTLOAD.SYS

build/cpuio/cpuio.o: cpuio/cpuio.c
	gcc -m32 -c cpuio/cpuio.c -o ./build/cpuio/cpuio.o -nostdlib -fno-pic -g

build/floppy.img: build/BOOTLOAD.SYS build/boot.bin build/bootloader-writer/bootloader-write
	dd if=/dev/zero of=build/floppy.img bs=512 count=2880
	mkdosfs -F 12 build/floppy.img

	./build/bootloader-writer/bootloader-write --image ./build/floppy.img --bootloader ./build/boot.bin --fs fat12

	sudo losetup /dev/loop0 ./build/floppy.img
	mkdir ./build/floppy
	sudo mount /dev/loop0 ./build/floppy
	sudo cp ./build/BOOTLOAD.SYS ./build/floppy/BOOTLOAD.SYS
	sudo umount ./build/floppy
	sudo losetup -d /dev/loop0
	rm -rf ./build/floppy

build/fs/floppy.o: fs/floppy.c
	gcc -m32 -c fs/floppy.c -o ./build/fs/floppy.o -nostdlib -fno-pic -g

build/screen/screen.o: screen/screen.c
	gcc -m32 -c screen/screen.c -o ./build/screen/screen.o -nostdlib -fno-pic -g

build/screen/stdio.o: screen/stdio.c
	gcc -m32 -c screen/stdio.c -o ./build/screen/stdio.o -nostdlib -fno-pic -g

build/screen/panic.o: screen/panic.c
	gcc -m32 -c screen/panic.c -o ./build/screen/panic.o -nostdlib -fno-pic -g

build/interrupts.o: interrupts.c
	gcc -m32 -c interrupts.c -o ./build/interrupts.o -nostdlib -fno-pic -g -mgeneral-regs-only

build/ps2/keyboard.o: ps2/keyboard.c
	gcc -m32 -c ps2/keyboard.c -o ./build/ps2/keyboard.o -nostdlib -fno-pic -g -mgeneral-regs-only

clean:
	rm -rf ./build
	rm -rf ./debug
	mkdir ./build
	mkdir ./build/bootloader-writer
	mkdir ./build/screen
	mkdir ./build/cpuio
	mkdir ./build/fs
	mkdir ./build/ps2
	mkdir ./debug

