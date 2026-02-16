#ifndef FLOPPY_H
#define FLOPPY_H

#include <stdint.h>

#define FLOPPY_SECTOR_SIZE 512
#define FLOPPY_SECTORS_PER_TRACK 18
#define FLOPPY_HEADS 2
#define FLOPPY_CYLINDERS 80

// See /boot2.asm
extern uint8_t BOOT_DISK;

struct CHS {
	uint8_t cylinder;
	uint8_t head;
	uint8_t sector;
};

enum FloppyRegisters
{
	STATUS_REGISTER_A		= 0x3F0, // read-only
	STATUS_REGISTER_B		= 0x3F1, // read-only
	DIGITAL_OUTPUT_REGISTER		= 0x3F2,
	TAPE_DRIVE_REGISTER		= 0x3F3,
	MAIN_STATUS_REGISTER		= 0x3F4, // read-only
	DATARATE_SELECT_REGISTER	= 0x3F4, // write-only
	DATA_FIFO			= 0x3F5,
	DIGITAL_INPUT_REGISTER		= 0x3F7, // read-only
	CONFIGURATION_CONTROL_REGISTER	= 0x3F7  // write-only
};

inline static void fdc_get_chs(int lba, struct CHS *chs)
{
	chs->head = (lba % (FLOPPY_HEADS * FLOPPY_SECTORS_PER_TRACK)) / FLOPPY_SECTORS_PER_TRACK;
	chs->cylinder = lba / (FLOPPY_HEADS * FLOPPY_SECTORS_PER_TRACK);
	chs->sector = (lba % FLOPPY_SECTORS_PER_TRACK + 1);
}

inline static int fdc_get_lba(struct CHS *chs)
{
	return (chs->cylinder * FLOPPY_HEADS + chs->head) * FLOPPY_SECTORS_PER_TRACK + (chs->sector - 1);
}

void fdc_init();

int fdc_read_sector(int drive, struct CHS *chs, int sector_count, uint8_t *buffer);
int fdc_write_sector(int drive, struct CHS *chs, int sector_count, uint8_t *buffer);

#endif
