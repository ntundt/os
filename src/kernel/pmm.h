#ifndef PMM_H
#define PMM_H

#include <stdint.h>

void pmm_init(void);

uint32_t pmm_get_bitmap_size(void);

void *pmm_alloc(void);

void pmm_free(void *page);

#endif
