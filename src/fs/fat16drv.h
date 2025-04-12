#ifndef FAT16DRV_H
#define FAT16DRV_H

#include <stdint.h>
#include <stdbool.h>

#include "fs/vfs.h"
#include "fs/fat16.h"

blkdevdescr_t fat16_blkdev(uint8_t fdc, uint8_t drive_num);
void fat16_lsdir(blkdevdescr_t *blkdev, const char *dir, struct fat16_dir *buffer, size_t capacity, bool *finished);
void fat16_fopen();
void fat16_fseek();
void fat16_fread();
void fat16_fclose();

#endif