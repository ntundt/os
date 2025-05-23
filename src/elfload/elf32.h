#ifndef ELF32_H
#define ELF32_h

#include <stdint.h>

typedef void* Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

#define ET_NONE 0 // No file type / No Machine

/**
 * Elf32_Ehdr.e_ident ELF header indexes
 */
#define EI_MAG0 0 // File identification
#define EI_MAG1 1 // File identification
#define EI_MAG2 2 // File identification
#define EI_MAG3 3 // File identification
#define EI_CLASS 4 // File class
#define EI_DATA 5 // Data encoding
#define EI_VERSION 6 // File version
#define EI_PAD 7 // Start of padding bytes
#define EI_NIDENT 16 // Size of e_ident[]

/**
 * Elf32_Ehdr.e_ident[EI_CLASS]
 */
#define CLASSNONE 0 // Invalid class
#define CLASS32 1 // 32-bit objects
#define CLASS64 2 // 64-bit objects

/**
 * Elf32_Ehdr.e_ident[EI_DATA]
 */
#define ELFDATANONE 0 // Invalid data encoding
#define ELFDATA2LSB 1 // 2's complement, with the LSB at the lowest address.
#define ELFDATA2MSB 2 // 2's complement, with the LSB at the highest address.

typedef struct {
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf32_Ehdr;

/**
 * e_type
 */
// ET_NONE 0 // No file type
#define ET_REL 1 // Relocatable file
#define ET_EXEC 2 // Executable file
#define ET_DYN 3 // Shared object file
#define ET_CORE 4 // Core file
#define ET_LOPROC 0xff00 // Processor specific
#define ET_HIPROC 0xffff // Processor specific

/**
 * e_machine
 */
// ET_NONE 0 // No machine
#define EM_M32 1 // AT&T WE 32100
#define EM_SPARC 2 // SPARC
#define EM_386 3 // Intel Architecture
#define EM_68K 4 // Motorola 68000
#define EM_88K 5 // Motorola 88000
#define EM_860 7 // Intel 80860
#define EM_MIPS 8 // MIPS RS3000 Big-Endian
#define EM_MIPS_RS4_BE 10 // MIPS RS4000 Big-Endian
#define RESERVED 11 //11-16 Reserved for future use

/**
 * e_version
 */
#define EV_NONE 0 // Invalid version
#define EV_CURRENT 1 // Current version

/**
 * Special section indexes
 */
#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff

typedef struct {
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} Elf32_Shdr;

/**
 * Section types
 */
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

/**
 * Section atrribute flags
 */
#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC 0xf0000000

typedef struct {
	Elf32_Word st_name;
	Elf32_Addr st_value;
	Elf32_Word st_size;
	unsigned char st_info;
	unsigned char st_other;
	Elf32_Half st_shndx;
} Elf32_Sym;

/**
 * st_info manipulation
 */
#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

/**
 * Symbol binding, ELF32_ST_BIND
 */
#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2
#define STB_LOPROC 13
#define STB_HIPROC 15

/**
 * Symbol types, ELF32_ST_TYPE
 */
#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4
#define STT_LOPROC 13
#define STT_HIPROC 15

typedef struct {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
} Elf32_Rel;

typedef struct {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
	Elf32_Sword r_addend;
} Elf32_Rela;

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

typedef struct {
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf32_Phdr;

/**
 * Segment Types, p_type
 */
#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff

#endif
