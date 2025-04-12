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
static uint8_t header_sector[BLOCK_SIZE];
static uint32_t sector_loaded = INVALID_SECTOR;
static uint8_t buffer[BLOCK_SIZE];

blkdevdescr_t fat16_blkdev(uint8_t fdc, uint8_t drive_num)
{
    return (blkdevdescr_t){ .fdc = fdc, .drive_num = drive_num };
}

static void ensure_header_sector_loaded(blkdevdescr_t *blkdev)
{
    if (header_sector_loaded)
        return;

    struct CHS chs;
    fdc_get_chs(0, &chs);
    if (fdc_read_sector(blkdev->drive_num, &chs, 1, header_sector) == 0) {
        header_sector_loaded = true;
        return;
    }

    panic("fdc_read_sector returned nonzero value");
}

static inline struct fat16_boot_sector *header(blkdevdescr_t *blkdev)
{
    ensure_header_sector_loaded(blkdev);
    return (struct fat16_boot_sector *)header_sector;
}

static void *get_offset_ptr(blkdevdescr_t *blkdev, size_t offset)
{
    const size_t needed_sector = offset / BLOCK_SIZE;

    if (needed_sector == sector_loaded)
        return &(buffer[offset % BLOCK_SIZE]);

    struct CHS chs;
    fdc_get_chs(needed_sector, &chs);
    if (fdc_read_sector(blkdev->drive_num, &chs, 1, buffer) != 0) {
        panic("fdc_read_sector returned nonzero value");
    }

    return &(buffer[offset % BLOCK_SIZE]);
}

static bool is_fat12(blkdevdescr_t *blkdev)
{
    return memcmp(header(blkdev)->system_id, "FAT12", 5) == 0;
}

void fat16_lsdir(blkdevdescr_t *blkdev, const char *dir, struct fat16_dir *buffer, size_t capacity, bool *finished)
{
    bool f12 = is_fat12(blkdev);
    panic("test");
}
