#ifndef ELF32_LOAD_H
#define ELF32_LOAD_H

#include <stdint.h>

/**
 * Load elf into memory
 * @returns address to jump to
 */
void *elfload(void* elfbuffer, size_t elfbuffer_size);

#endif
