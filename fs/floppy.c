#include "floppy.h"

#include "../cpuio/cpuio.h"

void floppy_get_chs(int lba, struct CHS *chs)
{
	chs->cylinder = lba / (FLOPPY_HEADS * FLOPPY_SECTORS_PER_TRACK);
	chs->head = (lba % (FLOPPY_HEADS * FLOPPY_SECTORS_PER_TRACK)) / FLOPPY_SECTORS_PER_TRACK;
	chs->sector = (lba % (FLOPPY_HEADS * FLOPPY_SECTORS_PER_TRACK)) % FLOPPY_SECTORS_PER_TRACK + 1;
}

int floppy_get_lba(struct CHS *chs)
{
	return (chs->cylinder * FLOPPY_HEADS + chs->head) * FLOPPY_SECTORS_PER_TRACK + (chs->sector - 1);
}

int floppy_controller_reinit()
{
	outb(0x3f2, 0x10);

	while ((inb(0x3f4) & 0xc0) != 0x80);

	uint8_t version = inb(0x3f7);

	if (version != 0x90) {
		return -1;
	}

	outb(0x3f7, 0x00);
	outb(0x3f7, 0x00);
	outb(0x3f7, 0x00);
	outb(0x3f7, 0x00);
	
	outb(0x3f2, 0x08);

	while ((inb(0x3f4) & 0xc0) != 0x80);

	uint8_t status = inb(0x3f7);

	if (status != 0x80) {
		return -2;
	}

	outb(0x3f7, 0x03);
	outb(0x3f7, 0x0f);
	outb(0x3f7, 0x0f);

	outb(0x3f2, 0x07);

	while ((inb(0x3f4) & 0xc0) != 0x80);

	status = inb(0x3f7);

	if (status != 0x20) {
		return -3;
	}

	return 0;
}

int floppy_read_sector(int drive, struct CHS *chs, int sector_count, uint8_t *buffer)
{
	outb(DIGITAL_OUTPUT_REGISTER, drive);

	outb(0x3f5, chs->head);

	outb(0x3f4, chs->cylinder);

	outb(0x3f5, chs->sector);

	outb(0x3f5, sector_count);

	outb(0x3f2, 0x46);

	while ((inb(0x3f4) & 0xc0) != 0x80);

	for (int i = 0; i < 512; i++) {
		buffer[i] = inb(0x3f7);
	}

	return 0;
}

int floppy_write_sector(int drive, struct CHS *chs, int sector_count, uint8_t *buffer)
{
	outb(0x3f2, drive);

	outb(0x3f5, chs->head);

	outb(0x3f4, chs->cylinder);

	outb(0x3f5, chs->sector);

	outb(0x3f5, sector_count);

	outb(0x3f2, 0x45);

	while ((inb(0x3f4) & 0xc0) != 0x80);

	for (int i = 0; i < 512; i++) {
		outb(0x3f7, buffer[i]);
	}

	return 0;
}
