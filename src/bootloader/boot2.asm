; Bootloader stage 2
[bits 16]

global _start
_start:
	mov [BOOT_DISK], dl

	CODE_SEG equ GDT_code - GDT_start
	DATA_SEG equ GDT_data - GDT_start
	
	mov al, 'A'
	mov ah, 0x0f
	mov bx, 0xb800
	mov ds, bx
	mov di, 0x0
	mov word [ds:di], ax

	cli
	xor ax, ax
	mov ds, ax
	lgdt [GDT_descriptor]

	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp CODE_SEG:start_protected_mode

[BITS 32]
extern bootloader_main

start_protected_mode:
	mov ax, DATA_SEG
	mov ss, ax

	mov esp, 0x200000

	mov al, 'B'
	mov ah, 0x0f
	mov [0xb8000], ax

	mov al, 'r'
	mov ah, 0x0f
	mov [0xb8002], ax

	mov al, 'u'
	mov ah, 0x0f
	mov [0xb8004], ax

	mov al, 'h'
	mov ah, 0x0f
	mov [0xb8006], ax

	mov dl, [BOOT_DISK]
	call bootloader_main

	hlt

BOOT_DISK: db 0

GDT_start:
	GDT_null:
		dd 0x0
		dd 0x0
	
	GDT_code:
		dw 0xffff
		dw 0x0
		db 0x0
		db 0b10011010
		db 0b11001111
		db 0x0
	
	GDT_data:
		dw 0xffff
		dw 0x0
		db 0x0
		db 0b10010010
		db 0b11001111
		db 0x0

	GDT_end:

GDT_descriptor:
	dw GDT_end - GDT_start - 1
	dd GDT_start