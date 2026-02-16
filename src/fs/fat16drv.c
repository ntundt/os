/**
 * A wonky and hacky bootloader FAT12/16 driver to find the kernel on the drive.
 * This will be improved later.
 */

#include <stdbool.h>

#include "kernel/kernel_stdlib.h"

#include "fs/fat16.h"
#include "vfs.h"
#include "fs/fat16drv.h"
#include "screen/panic.h"
#include "screen/stdio.h"
#include "utils/utils.h"

#include "fs/floppy.h"

#define BLOCK_SIZE 512
#define INVALID_SECTOR -1

static uint8_t boot_sector_buffer[BLOCK_SIZE];

static uint32_t sector_loaded = INVALID_SECTOR;
static uint8_t sector_buffer[BLOCK_SIZE];

static uint8_t fat[BLOCK_SIZE * 32];

static size_t fat16_fat_get_next_cluster(blkdev_t *blkdev, size_t cluster_num);

blkdev_t fat16_blkdev(uint8_t fdc, uint8_t drive_num)
{
	return (blkdev_t){ .fdc = fdc, .drive_num = drive_num };
}

static void fat16_ensure_bootsect_loaded(blkdev_t *blkdev)
{
	static bool header_sector_loaded = false;
	if (header_sector_loaded)
		return;

	struct CHS chs;
	fdc_get_chs(0, &chs);
	if (fdc_read_sector(blkdev->drive_num, &chs, 1, boot_sector_buffer) != 0)
		panic("fdc_read_sector returned nonzero value");
	
	header_sector_loaded = true;
	return;
}

static inline struct fat16_boot_sector *fat16_get_boot_sector(blkdev_t *blkdev)
{
	fat16_ensure_bootsect_loaded(blkdev);
	return (struct fat16_boot_sector *)boot_sector_buffer;
}

static void *get_offset_ptr(blkdev_t *blkdev, size_t offset)
{
	const size_t needed_sector = offset / BLOCK_SIZE;

	if (needed_sector == sector_loaded)
		return &(sector_buffer[offset % BLOCK_SIZE]);

	struct CHS chs;
	fdc_get_chs(needed_sector, &chs);
	if (fdc_read_sector(blkdev->drive_num, &chs, 1, sector_buffer) != 0)
		panic("fdc_read_sector returned nonzero value");

	sector_loaded = needed_sector;

	return sector_buffer + offset % BLOCK_SIZE;
}

static bool is_fat12(blkdev_t *blkdev)
{
	return memcmp(fat16_get_boot_sector(blkdev)->system_id, "FAT12", 5) == 0;
}

static bool is_fat16(blkdev_t *blkdev)
{
	return memcmp(fat16_get_boot_sector(blkdev)->system_id, "FAT16", 5) == 0;
}

#define LOADDIR_SUCCESS 0
#define LOADDIR_FAIL -1

static inline size_t get_root_dir_sectors(blkdev_t *blkdev)
{
	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	return bs->root_entries * sizeof(struct fat16_direntry) / bs->bytes_per_sector;
}

static inline size_t get_occupied_bytes_at_beginning(blkdev_t *blkdev)
{
	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	return (bs->reserved_sectors + bs->number_of_fats * bs->sectors_per_fat + get_root_dir_sectors(blkdev)) * bs->bytes_per_sector;
}

static inline bool is_at_cluster_boundary(blkdev_t *blkdev, size_t offset)
{
	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	return (offset - get_occupied_bytes_at_beginning(blkdev)) % (bs->bytes_per_sector * bs->sectors_per_cluster) == 0;
}

static inline size_t get_cluster_num(blkdev_t *blkdev, size_t offset)
{
	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	return (offset - get_occupied_bytes_at_beginning(blkdev)) / (bs->bytes_per_sector * bs->sectors_per_cluster);
}

static inline size_t get_cluster_offset(blkdev_t *blkdev, size_t cluster_num)
{
	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	return get_occupied_bytes_at_beginning(blkdev)
		+ (cluster_num - 2) * bs->sectors_per_cluster * bs->bytes_per_sector;
}

static inline size_t get_next_sector_boundary_offset(blkdev_t *blkdev, size_t offset)
{
	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	return (offset / bs->bytes_per_sector + 1) * bs->bytes_per_sector;
}

static inline size_t get_next_cluster_boundary_offset(blkdev_t *blkdev, size_t offset)
{
	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	return ((offset - get_occupied_bytes_at_beginning(blkdev)) / (bs->bytes_per_sector * bs->sectors_per_cluster) + 1)
		* (bs->bytes_per_sector * bs->sectors_per_cluster) + get_occupied_bytes_at_beginning(blkdev);
}

static inline size_t get_next_offset(blkdev_t *blkdev, size_t offset)
{
	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	size_t next_sector_boundary = get_next_sector_boundary_offset(blkdev, offset);
	size_t next_cluster_boundary = get_next_cluster_boundary_offset(blkdev, offset);

	if (offset + bs->bytes_per_sector >= next_cluster_boundary)
		return fat16_fat_get_next_cluster(blkdev, get_cluster_num(blkdev, offset));
	
	return next_sector_boundary;
}

static bool offsets_within_same_sector(blkdev_t *blkdev, size_t o1, size_t o2)
{
	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	return o1 / bs->bytes_per_sector == o2 / bs->bytes_per_sector;
}

static size_t fat16_fread_root(void *dest, size_t size, size_t n, fat16_file *file)
{
	struct fat16_boot_sector *bs = fat16_get_boot_sector(&file->blkdev);
	size_t root_dir_start_offset = (bs->reserved_sectors + bs->sectors_per_fat * bs->number_of_fats)
		* bs->bytes_per_sector;
	
	size_t read_start_offset = root_dir_start_offset + file->current_offset;

	size_t bytes_read = 0;

	while (bytes_read < size * n && bytes_read < file->file_length) {
		memcpy(((uint8_t *)dest) + bytes_read, get_offset_ptr(&file->blkdev, read_start_offset + bytes_read), size);
		bytes_read += size;
		file->current_offset += size;
	}

	return bytes_read;
}

size_t fat16_fread(void *dest, size_t size, size_t n, fat16_file *file)
{
	if (file->is_root_dir)
		return fat16_fread_root(dest, size, n, file);
	
	/**
	 * v stands for virtual, p for physical
	 */

	size_t total_size_left = size * n;
	size_t p_offset = get_cluster_offset(&file->blkdev, file->start_cluster);
	size_t v_read_offset = 0;
	bool v_read_offset_found = false;
	while (total_size_left > 0 && v_read_offset < file->file_length) {
		size_t p_file_current_offset = p_offset + file->current_offset - v_read_offset;
		if (offsets_within_same_sector(&file->blkdev, p_offset, p_file_current_offset) && !v_read_offset_found) {
			v_read_offset += p_file_current_offset - p_offset;
			p_offset = p_file_current_offset;
			v_read_offset_found = true;
		}

		size_t p_next_sector_boundary = get_next_sector_boundary_offset(&file->blkdev, p_offset);
		size_t p_next_cluster_boundary = get_next_cluster_boundary_offset(&file->blkdev, p_offset);
		
		size_t p_needed_offset = p_offset + total_size_left;

		// Offset of the end of readable space
		size_t p_readable_end_offset = LEAST(LEAST(p_next_sector_boundary, p_next_cluster_boundary), p_needed_offset);

		size_t bytes_to_read = LEAST(p_readable_end_offset - p_offset, total_size_left);
		if (v_read_offset < file->current_offset)
			bytes_to_read = LEAST(bytes_to_read, p_file_current_offset - p_offset);

		if (v_read_offset >= file->current_offset) {
			memcpy(dest, get_offset_ptr(&file->blkdev, p_offset), bytes_to_read);
			total_size_left -= bytes_to_read;
		}
		
		v_read_offset += bytes_to_read;
		p_offset = get_next_offset(&file->blkdev, p_offset);
	}
	file->current_offset = LEAST(file->file_length, file->current_offset + size * n);
	return size * n - total_size_left;
}

static fat16_file fat16_fopen_root(blkdev_t *blkdev)
{
	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	return (fat16_file){
		.is_root_dir = true,
		.start_cluster = 0, 
		.file_length = bs->root_entries * sizeof(struct fat16_direntry),
		.current_offset = 0,
		.blkdev = *blkdev,
	};
}

void fat16_fopen(blkdev_t *blkdev, const char *path, fat16_file **f)
{
	fat16_file current = fat16_fopen_root(blkdev);
	while (path != NULL && *path != '\0') {
		if (*path == '/') {
			path += 1;
			continue;
		}
	
		char direntry_name_buffer[12] = "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20";
	
		const char *end = strstr(path, "/");
		if (end == NULL)
			end = path + strlen(path);
		
		memcpy(direntry_name_buffer, path, (size_t)(end - path));
	
		struct fat16_direntry direntry;

		while (fat16_fread(&direntry, sizeof(struct fat16_direntry), 1, &current)) {
			if (direntry.name[0] == '\0') {
				*f = NULL;
			}
			
			char curr_direntry_name_buffer[12] = "\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20";
			memcpy(curr_direntry_name_buffer, direntry.name, 11);

			if (strcmp(curr_direntry_name_buffer, direntry_name_buffer) == 0) {
				current = (fat16_file){
					.is_root_dir = false,
					.blkdev = *blkdev,
					.current_offset = 0,
					.file_length = (direntry.attributes & FAT16_DIRENTRY_FLAG_DIRECTORY) == 0
						? current.file_length : FILE_INVALID_LENGTH,
					.start_cluster = direntry.first_cluster_low,
				};

				path = strstr(path, "/");
				break;
			}
		}
	}

	**f = current;
}

static int fat16_fat_ensure_loaded(blkdev_t *blkdev)
{
	static bool fat_loaded = false;
	if (!fat_loaded)
		fat_loaded = true;
	else
		return LOADDIR_SUCCESS;

	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	const size_t fat_offset = bs->reserved_sectors * bs->bytes_per_sector;
	
	for (int i = 0; i < bs->sectors_per_fat; ++i) {
		const void *sector_ptr = get_offset_ptr(blkdev, fat_offset
			+ i * bs->bytes_per_sector);
		memcpy(fat + i * bs->bytes_per_sector, sector_ptr, bs->bytes_per_sector);
	}

	return LOADDIR_SUCCESS;
}

static void *fat16_fat_get_offset(blkdev_t *blkdev, size_t offset)
{
	int ret = fat16_fat_ensure_loaded(blkdev);
	assert(ret == LOADDIR_SUCCESS);

	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	offset -= bs->bytes_per_sector * bs->reserved_sectors;
	return &(fat[offset]);
}

/**
 * cluster_num is the number of cluster, next cluster to which is needed.
 * Starts at 1.
 */
static size_t fat16_fat_get_next_cluster(blkdev_t *blkdev, size_t cluster_num) {
	int ret = fat16_fat_ensure_loaded(blkdev);
	assert(ret == LOADDIR_SUCCESS);

	if (is_fat12(blkdev)) {
		uint16_t *tmp = fat16_fat_get_offset(blkdev, (cluster_num - 1) * 3 / 2);
		if (cluster_num % 2 == 0) {
			return *tmp & 0x0FFF;
		} else {
			return (*tmp & 0xFFF0) >> 4;
		}
	} else if (is_fat16(blkdev))
		return *(uint16_t *)fat16_fat_get_offset(blkdev, cluster_num * 2);
	
	assert(false && "FAT filesystem subtype not supported");
}

void fat16_lsdir(blkdev_t *blkdev, const char *dir, uint32_t start_entry,
	struct fat16_direntry *buffer, size_t *capacity, bool *finished)
{
	// TODO: set finished
	(void) finished;

	struct fat16_boot_sector *bs = fat16_get_boot_sector(blkdev);
	
	size_t dir_offset = (bs->reserved_sectors
		+ bs->number_of_fats * bs->sectors_per_fat) * bs->bytes_per_sector;

	fat16_file f;
	fat16_file *res = &f;
	fat16_fopen(blkdev, dir, &res);

	if (res == NULL)
		panic("could not find the dir");

	uint32_t actual_dir_count = 0;
	while (actual_dir_count < start_entry + *capacity) {
		if (actual_dir_count < start_entry) {
			dir_offset += sizeof(struct fat16_direntry);
			continue;
		}

		struct fat16_direntry *dirent = get_offset_ptr(blkdev, dir_offset);

		if (dirent->name[0] == '\0') break;

		memcpy(buffer + (actual_dir_count - start_entry), dirent,
			sizeof(struct fat16_direntry));

		dir_offset += sizeof(struct fat16_direntry);
		actual_dir_count++;
	}
	*capacity = actual_dir_count;
}
