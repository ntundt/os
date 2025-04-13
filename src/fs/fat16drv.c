#include <stdbool.h>

#include "bootloader/bootloader_stdlib.h"

#include "fs/fat16.h"
#include "vfs.h"
#include "screen/panic.h"
#include "screen/stdio.h"

#include "fs/floppy.h"

#define BLOCK_SIZE 512
#define INVALID_SECTOR -1

static bool header_sector_loaded = false;
static uint8_t boot_sector_buffer[BLOCK_SIZE];
static uint32_t sector_loaded = INVALID_SECTOR;
static uint8_t sector_buffer[BLOCK_SIZE];

blkdevdescr_t fat16_blkdev(uint8_t fdc, uint8_t drive_num)
{
    return (blkdevdescr_t){ .fdc = fdc, .drive_num = drive_num };
}

static void fat16_ensure_bootsect_loaded(blkdevdescr_t *blkdev)
{
    if (header_sector_loaded)
        return;

    struct CHS chs;
    fdc_get_chs(0, &chs);
    if (fdc_read_sector(blkdev->drive_num, &chs, 1, boot_sector_buffer) == 0) {
        header_sector_loaded = true;
        return;
    }

    panic("fdc_read_sector returned nonzero value");
}

static inline struct fat16_boot_sector *fat16bootsect(blkdevdescr_t *blkdev)
{
    fat16_ensure_bootsect_loaded(blkdev);
    return (struct fat16_boot_sector *)boot_sector_buffer;
}

static void *get_offset_ptr(blkdevdescr_t *blkdev, size_t offset)
{
    const size_t needed_sector = offset / BLOCK_SIZE;

    if (needed_sector == sector_loaded)
        return &(sector_buffer[offset % BLOCK_SIZE]);

    struct CHS chs;
    fdc_get_chs(needed_sector, &chs);
    if (fdc_read_sector(blkdev->drive_num, &chs, 1, sector_buffer) != 0) {
        panic("fdc_read_sector returned nonzero value");
    }

    sector_loaded = needed_sector;

    return sector_buffer + offset % BLOCK_SIZE;
}

static bool is_fat12(blkdevdescr_t *blkdev)
{
    return memcmp(fat16bootsect(blkdev)->system_id, "FAT12", 5) == 0;
}

static bool is_fat16(blkdevdescr_t *blkdev)
{
    return memcmp(fat16bootsect(blkdev)->system_id, "FAT16", 5) == 0;
}

void fat16_lsdir(blkdevdescr_t *blkdev, const char *dir, uint32_t start_entry,
    struct fat16_direntry *buffer, size_t *capacity, bool *finished)
{
    bool f12 = is_fat12(blkdev);

    struct fat16_boot_sector *bs = fat16bootsect(blkdev);
    printfd("bytes per sector = %d, sectors per cluster = %d\n",
        (int32_t)bs->bytes_per_sector, (int32_t)bs->sectors_per_cluster);
    printfd("reserved sectors = %d, number of FATS = %d\n",
        (int32_t)bs->reserved_sectors, (int32_t)bs->number_of_fats);
    printfd("root entries = %d, total sectors = %d\n",
        (int32_t)bs->root_entries, (int32_t)bs->total_sectors);
    printfd("media descriptor = %d, sectors per FAT = %d\n",
        (int32_t)bs->media_descriptor, (int32_t)bs->sectors_per_fat);
    printfd("sectors per track = %d, number of heads = %d\n",
        (int32_t)bs->sectors_per_track, (int32_t)bs->number_of_heads);
    printfd("hidden sectors = %d, large sectors = %d\n",
        (int32_t)bs->hidden_sectors, (int32_t)bs->large_sectors);
    printfd("reserved sectors = %d, number of FATS = %d\n",
        (int32_t)bs->drive_number, (int32_t)bs->current_head);
    printfd("boot signature = %d, volume ID = %d\n",
        (int32_t)bs->boot_signature, (int32_t)bs->volume_id);
    
    size_t dir_offset = (bs->reserved_sectors
       + bs->number_of_fats * bs->sectors_per_fat) * bs->bytes_per_sector;
    printfd("dir_offset = %x\n", dir_offset);

    for (int i = 0; i < start_entry + *capacity; ++i) {
        if (i < start_entry) {
            dir_offset += sizeof(struct fat16_direntry);
            continue;
        }

        struct fat16_direntry *dirent = get_offset_ptr(blkdev, dir_offset);

        memcpy(buffer + (i - start_entry), dirent, sizeof(struct fat16_direntry));

        dir_offset += sizeof(struct fat16_direntry);
    }
}
