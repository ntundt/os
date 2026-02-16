ENTRY(_start)

KERNEL_VIRT_BASE = 0xC0100000;
KERNEL_PHYS_BASE = 0x00100000;

SECTIONS
{
    /* Kernel virtual address */
    . = KERNEL_VIRT_BASE;

    _kstart = .;

    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRT_BASE + KERNEL_PHYS_BASE)
    {
        *(.multiboot)
        *(.text*)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRT_BASE + KERNEL_PHYS_BASE)
    {
        *(.rodata*)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRT_BASE + KERNEL_PHYS_BASE)
    {
        *(.data*)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VIRT_BASE + KERNEL_PHYS_BASE)
    {
        *(COMMON)
        *(.bss*)
    }

    _kend = .;
}
