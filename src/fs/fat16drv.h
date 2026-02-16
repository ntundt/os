#ifndef FAT16DRV_H
#define FAT16DRV_H

#include <stdint.h>
#include <stdbool.h>

#include "fs/vfs.h"
#include "fs/fat16.h"

#define FILE_INVALID_LENGTH UINT32_MAX

typedef struct {
	bool is_root_dir;
	size_t start_cluster;
	size_t current_offset;
	size_t file_length;
	blkdev_t blkdev;
} fat16_file;

blkdev_t fat16_blkdev(uint8_t fdc, uint8_t drive_num);
void fat16_lsdir(blkdev_t *blkdev, const char *dir, uint32_t start_entry,
	struct fat16_direntry *buffer, size_t *capacity, bool *finished);
void fat16_fopen(blkdev_t *blkdev, const char *path, fat16_file **f);
void fat16_fseek();
size_t fat16_fread(void *dest, size_t size, size_t n, fat16_file *file);
void fat16_fclose();

#endif
