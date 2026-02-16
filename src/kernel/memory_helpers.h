#ifndef MEMORY_HELPERS_H
#define MEMORY_HELPERS_H

#define ALIGN_DOWN(addr, segment_size) ((void *)(((uint32_t)(addr)) & ~((segment_size) - 1)))
#define ALIGN_UP(addr, segment_size) ((void*)(((uint32_t)(addr) + (segment_size) - 1) & ~((segment_size) - 1)))

#endif
