CC = gcc
DEBUG =
INCLUDE_PATH = -I./src/
CC_OPTIONS = -Wall -Wextra ${INCLUDE_PATH} -fno-builtin ${DEBUG} -nostdlib -fno-pic -g
QEMU_PATH_I386 = qemu-system-i386
QEMU_TERMINAL_OPTIONS = -cdrom ./build/img.iso -d int,cpu_reset -m 2G
BUILD_DIR = ./build
DEBUG_DIR = ./debug
DEBUG_FILE = vmlinuz.debug
TOSTRIPFILE = ./build/vmlinuz

build: ./build/img.iso

run: ./build/img.iso
	$(QEMU_PATH_I386) $(QEMU_TERMINAL_OPTIONS)

debug: ./build/img.iso
	$(QEMU_PATH_I386) $(QEMU_TERMINAL_OPTIONS) -s -S &
	objcopy --only-keep-debug "${TOSTRIPFILE}" "${DEBUG_DIR}/${DEBUG_FILE}"
	sleep 1
	gdb -x ./boot.gdb

build/cpuio/cpuio.o: ./src/cpuio/cpuio.c
	$(CC) -m32 -c ./src/cpuio/cpuio.c -o ./build/cpuio/cpuio.o ${CC_OPTIONS}

build/fs/fat16drv.o: ./src/fs/fat16drv.c
	$(CC) -m32 -c ./src/fs/fat16drv.c -o ./build/fs/fat16drv.o ${CC_OPTIONS}

build/fs/floppy.o: ./src/fs/floppy.c
	$(CC) -m32 -c ./src/fs/floppy.c -o ./build/fs/floppy.o ${CC_OPTIONS}

build/vmlinuz: build/kernel/kernel_main.o kernel.ld \
	build/screen/screen.o build/cpuio/cpuio.o build/kernel/kernel_stdlib.o \
	build/screen/stdio.o build/screen/panic.o build/interrupts.o \
	build/fs/floppy.o build/ps2/keyboard.o build/fs/fat16drv.o \
	build/kernel/multiboot.o build/kernel/bootstrap.o \
	build/kernel/gdt/gdt.o build/kernel/pmm.o build/kernel/vmem.o
	ld -m elf_i386 -T kernel.ld -z noexecstack -o build/vmlinuz \
		./build/kernel/kernel_main.o ./build/screen/screen.o ./build/cpuio/cpuio.o \
		./build/kernel/kernel_stdlib.o ./build/screen/stdio.o \
		./build/screen/panic.o ./build/interrupts.o ./build/fs/floppy.o \
		./build/ps2/keyboard.o ./build/fs/fat16drv.o ./build/kernel/multiboot.o \
		./build/kernel/bootstrap.o ./build/kernel/gdt/gdt.o build/kernel/pmm.o \
		./build/kernel/vmem.o

build/img.iso: ./build/vmlinuz
	mkdir -p build/isodir/boot/grub
	cp ./build/vmlinuz build/isodir/boot/vmlinuz
	cp grub.cfg build/isodir/boot/grub/grub.cfg
	grub-mkrescue -o build/img.iso build/isodir

build/kernel/gdt/gdt.o: ./src/kernel/gdt/gdt.c
	$(CC) -m32 -c ./src/kernel/gdt/gdt.c -o ./build/kernel/gdt/gdt.o ${CC_OPTIONS}

build/kernel/kernel_main.o: ./src/kernel/kernel_main.c
	$(CC) -m32 -c ./src/kernel/kernel_main.c -o ./build/kernel/kernel_main.o ${CC_OPTIONS}

build/kernel/kernel_stdlib.o: ./src/kernel/kernel_stdlib.c
	$(CC) -m32 -c ./src/kernel/kernel_stdlib.c -o ./build/kernel/kernel_stdlib.o ${CC_OPTIONS}

build/kernel/multiboot.o: ./src/kernel/multiboot.c
	$(CC) -m32 -c ./src/kernel/multiboot.c -o ./build/kernel/multiboot.o ${CC_OPTIONS}

build/kernel/bootstrap.o: ./src/kernel/bootstrap.asm
	nasm -f elf32 ./src/kernel/bootstrap.asm -o ./build/kernel/bootstrap.o

build/kernel/pmm.o: ./src/kernel/pmm.c
	$(CC) -m32 -c ./src/kernel/pmm.c -o ./build/kernel/pmm.o ${CC_OPTIONS}

build/kernel/vmem.o: ./src/kernel/vmem.c
	$(CC) -m32 -c ./src/kernel/vmem.c -o ./build/kernel/vmem.o ${CC_OPTIONS}

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
	mkdir ${BUILD_DIR}/kernel/gdt
	mkdir ${BUILD_DIR}/ps2
	mkdir ${BUILD_DIR}/screen
	mkdir ${DEBUG_DIR}
