CC = gcc
DEBUG = -DDEBUG
INCLUDE_PATH = -I./src/
CC_OPTIONS = -Werror ${INCLUDE_PATH} -fno-builtin ${DEBUG} -nostdlib -fno-pic -g
QEMU_PATH_I386 = /mnt/c/Apps/qemu/qemu-system-i386.exe
QEMU_TERMINAL_OPTIONS = -fda ./build/floppy.img
BUILD_DIR = ./build
DEBUG_DIR = ./debug
DEBUG_FILE = bootloader2.o.debug
TOSTRIPFILE = ./build/bootloader2.o

build: ./build/floppy.img

run: ./build/floppy.img
	$(QEMU_PATH_I386) $(QEMU_TERMINAL_OPTIONS)

debug: ./build/floppy.img
	$(QEMU_PATH_I386) $(QEMU_TERMINAL_OPTIONS) -s -S &
	objcopy --only-keep-debug "${TOSTRIPFILE}" "${DEBUG_DIR}/${DEBUG_FILE}"
	sleep 5
	gdb -x ./boot.gdb

build/bootloader/bootloader_stdlib.o: ./src/bootloader/bootloader_stdlib.c
	$(CC) -m32 -c ./src/bootloader/bootloader_stdlib.c -o ./build/bootloader/bootloader_stdlib.o ${CC_OPTIONS}

build/bootloader-writer/bootloader-write: ./src/bootloader-writer/bootloader-write.c
	$(CC) -o ./build/bootloader-writer/bootloader-write ./src/bootloader-writer/bootloader-write.c -g ${INCLUDE_PATH}

build/bootloader/boot.bin: ./src/bootloader/boot.asm
	nasm ./src/bootloader/boot.asm -f bin -o ./build/bootloader/boot.bin

build/bootloader/boot2.o: ./src/bootloader/boot2.asm
	nasm -f elf32 ./src/bootloader/boot2.asm -o ./build/bootloader/boot2.o

build/bootloader/bootloader.o: ./src/bootloader/bootloader.c
	$(CC) -m32 -c ./src/bootloader/bootloader.c -o ./build/bootloader/bootloader.o -O0 ${CC_OPTIONS}

build/bootloader2.o: build/bootloader/bootloader.o build/bootloader/boot2.o bootload.sys.ld \
	build/screen/screen.o build/cpuio/cpuio.o build/bootloader/bootloader_stdlib.o \
	build/screen/stdio.o build/screen/panic.o build/interrupts.o \
	build/fs/floppy.o build/ps2/keyboard.o build/fs/fat16drv.o
	ld -m elf_i386 -T bootload.sys.ld -o ./build/bootloader/bootloader2.o ./build/bootloader/boot2.o \
		./build/bootloader/bootloader.o ./build/screen/screen.o ./build/cpuio/cpuio.o \
		./build/bootloader/bootloader_stdlib.o ./build/screen/stdio.o \
		./build/screen/panic.o ./build/interrupts.o ./build/fs/floppy.o \
		./build/ps2/keyboard.o ./build/fs/fat16drv.o

build/bootloader/BOOTLOAD.SYS: ./build/bootloader2.o
	objcopy -O binary -j .text -j .bss -j .data -j .rodata ./build/bootloader/bootloader2.o ./build/bootloader/BOOTLOAD.SYS

build/cpuio/cpuio.o: ./src/cpuio/cpuio.c
	$(CC) -m32 -c ./src/cpuio/cpuio.c -o ./build/cpuio/cpuio.o ${CC_OPTIONS}

build/floppy.img: ./build/bootloader/BOOTLOAD.SYS ./build/bootloader/boot.bin ./build/bootloader-writer/bootloader-write
	dd if=/dev/zero of=build/floppy.img bs=512 count=2880
	mkdosfs -F 12 ./build/floppy.img

	./build/bootloader-writer/bootloader-write --image ./build/floppy.img --bootloader ./build/bootloader/boot.bin --fs fat12

	echo "drive a: file=\"./build/floppy.img\"" > ./build/mtools.conf
	MTOOLSRC=./build/mtools.conf mcopy ./build/bootloader/BOOTLOAD.SYS a:/BOOTLOAD.SYS

build/fs/fat16drv.o: ./src/fs/fat16drv.c
	$(CC) -m32 -c ./src/fs/fat16drv.c -o ./build/fs/fat16drv.o ${CC_OPTIONS}

build/fs/floppy.o: ./src/fs/floppy.c
	$(CC) -m32 -c ./src/fs/floppy.c -o ./build/fs/floppy.o ${CC_OPTIONS}

build/screen/screen.o: ./src/screen/screen.c
	$(CC) -m32 -c ./src/screen/screen.c -o ./build/screen/screen.o ${CC_OPTIONS}

build/screen/stdio.o: ./src/screen/stdio.c
	$(CC) -m32 -c ./src/screen/stdio.c -o ./build/screen/stdio.o ${CC_OPTIONS}

build/screen/panic.o: ./src/screen/panic.c
	$(CC) -m32 -c ./src/screen/panic.c -o ./build/screen/panic.o ${CC_OPTIONS}

build/interrupts.o: ./src/interrupts.c
	$(CC) -m32 -c ./src/interrupts.c -o ./build/interrupts.o -mgeneral-regs-only ${CC_OPTIONS}

build/ps2/keyboard.o: ./src/ps2/keyboard.c
	$(CC) -m32 -c ./src/ps2/keyboard.c -o ./build/ps2/keyboard.o -mgeneral-regs-only ${CC_OPTIONS}

clean:
	rm -rf ${BUILD_DIR}
	rm -rf ${DEBUG_DIR}
	mkdir ${BUILD_DIR}
	mkdir ${BUILD_DIR}/bootloader-writer
	mkdir ${BUILD_DIR}/bootloader
	mkdir ${BUILD_DIR}/cpuio
	mkdir ${BUILD_DIR}/fs
	mkdir ${BUILD_DIR}/kernel
	mkdir ${BUILD_DIR}/ps2
	mkdir ${BUILD_DIR}/screen
	mkdir ${DEBUG_DIR}
