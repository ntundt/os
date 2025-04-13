target remote 192.168.100.28:1234
file ./debug/bootloader2.o.debug
layout src
set disassembly-flavor intel
set debug-file-directory ./debug

# uncomment to debug bootloader:
#set architecture i8086
#set tdesc filename target.xml
#break *0x7c00

b fs/fat16drv.c:88

alias da = x/8i ($cs << 4) + $eip
c
