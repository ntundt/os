#ifndef VFS_H
#define VFS_H

#include <stdint.h>

typedef struct {
	uint8_t fdc, drive_num;
} blkdevdescr_t;

void vfs();

#endif