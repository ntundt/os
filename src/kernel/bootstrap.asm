BITS 32

%define KERNEL_VIRT_BASE 0xC0000000
%define PAGE_PRESENT 0x1
%define PAGE_RW      0x2

section .multiboot
align 8
mb2_header_start:
    dd 0xE85250D6
    dd 0
    dd mb2_header_end - mb2_header_start
    dd -(0xE85250D6 + 0 + (mb2_header_end - mb2_header_start))

    align 8
    dd 6
    dd 8
    dd 0

    align 8
    dd 14
    dd 8
    dd 0

    align 8
    dd 15
    dd 8
    dd 0

    align 8
    dd 0
    dd 8
    dd 0
mb2_header_end:

section .bss
align 4096
page_directory:
    resd 1024

align 4096
page_table_low:
    resd 1024

align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top

    ; Save multiboot params
    mov esi, eax
    mov edi, ebx

    ; -------------------------
    ; Build page table (identity)
    ; -------------------------
    mov ecx, 1024
    xor ebx, ebx

.fill_pt:
    mov eax, ebx
    shl eax, 12
    or eax, PAGE_PRESENT | PAGE_RW
    mov [page_table_low - KERNEL_VIRT_BASE + ebx*4], eax
    inc ebx
    loop .fill_pt

    ; -------------------------
    ; Page directory entries
    ; -------------------------
    mov eax, page_table_low - KERNEL_VIRT_BASE
    or eax, PAGE_PRESENT | PAGE_RW

    ; PD[0] identity
    mov [page_directory - KERNEL_VIRT_BASE + 0*4], eax

    ; PD[768] higher half (3GB / 4MB)
    mov [page_directory - KERNEL_VIRT_BASE + 768*4], eax

    ; Set page directory recursively mapped at itself at 0xFFFFF000
    mov eax, page_directory - KERNEL_VIRT_BASE
    or eax, PAGE_PRESENT | PAGE_RW
    mov [page_directory - KERNEL_VIRT_BASE + 1023*4], eax

    ; -------------------------
    ; Enable paging
    ; -------------------------
    mov eax, page_directory - KERNEL_VIRT_BASE
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; Flush pipeline
    jmp higher_half_entry
    
    ; -------------------------
    ; Jump to higher half
    ; -------------------------
higher_half_entry:
    mov esp, stack_top

    push edi
    push esi

    mov eax, kernel_main
    call eax

.hang:
    cli
    hlt
    jmp .hang
