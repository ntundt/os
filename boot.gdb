target remote 127.0.0.1:1234
file ./build/vmlinuz
layout src
set disassembly-flavor intel
set debug-file-directory ./debug

# uncomment to debug bootloader:
#set architecture i8086
#set tdesc filename target.xml
#break *0x7c00

#b fs/fat16drv.c:88
b vm_init
#watch *(unsigned int*)0xc012d4b0 if *(unsigned int*)0xc012d4b0 == 0x0012c003

alias da = x/8i ($cs << 4) + $eip
c
