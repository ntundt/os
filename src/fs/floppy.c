#include "screen/stdio.h"
#include "screen/panic.h"
#include "floppy.h"
#include "cpuio/cpuio.h"
#include "interrupts.h"
#include "bootloader/bootloader_stdlib.h"

#define FLOPPY_CMD_READ		0x06
#define FLOPPY_CMD_WRITE	0x05
#define FLOPPY_CMD_RECALIBRATE 	0x07
#define FLOPPY_CMD_SENSE_INT   	0x08
#define FLOPPY_CMD_SEEK		0x0F

#define FLOPPY_MSR_RQM		0x80
#define FLOPPY_MSR_DIO		0x40

volatile int floppy_irq_triggered = 0;

void floppy_irq_handler(void *stack)
{
	(void) stack;
	floppy_irq_triggered = 1;
	outb(0x20, 0x20); // PIC EOI
}

static void floppy_wait_irq()
{
	while (!floppy_irq_triggered);
	floppy_irq_triggered = 0;
}

static void floppy_send_command(uint8_t cmd)
{
	while ((inb(MAIN_STATUS_REGISTER) & FLOPPY_MSR_RQM) == 0);
	outb(DATA_FIFO, cmd);
}

static void floppy_send_command_arg(uint8_t cmd, uint8_t arg)
{
	floppy_send_command(cmd);
	floppy_send_command(arg);
}

static uint8_t floppy_read_data()
{
	while ((inb(MAIN_STATUS_REGISTER) & (FLOPPY_MSR_RQM | FLOPPY_MSR_DIO)) != (FLOPPY_MSR_RQM | FLOPPY_MSR_DIO));
	return inb(DATA_FIFO);
}

static void floppy_write_data(uint8_t val)
{
	while ((inb(MAIN_STATUS_REGISTER) & (FLOPPY_MSR_RQM | FLOPPY_MSR_DIO)) != FLOPPY_MSR_RQM);
	outb(DATA_FIFO, val);
}

#define DMA_CHANNEL		 2
#define DMA_BUFFER_ADDR	 0x8000  // physical address
#define DMA_PAGE_PORT	   0x81	// Page register for channel 2
#define DMA_MASK_REG		0x0A
#define DMA_MODE_REG		0x0B
#define DMA_CLEAR_FF		0x0C
#define DMA_ADDR_CH2		0x04
#define DMA_COUNT_CH2	   0x05

void dma_floppy_prepare_read(uint32_t phys_addr, uint16_t length) {
	// TODO: account for length
	(void) length;

	uint8_t page = (phys_addr >> 16) & 0xFF;
	uint16_t offset = phys_addr & 0xFFFF;

	// Step 1: Mask channel 2
	outb(DMA_MASK_REG, 0x06);  // bit 2 (channel), bit 1 (mask), bit 0 = 0 (disable mask write)

	// Step 2: Clear flip-flop (internal address/data pointer toggle)
	outb(DMA_CLEAR_FF, 0xFF);

	// Step 3: Set address
	outb(DMA_ADDR_CH2, offset & 0xFF); // low byte
	outb(DMA_ADDR_CH2, (offset >> 8) & 0xFF); // high byte

	// Step 4: Clear flip-flop again before count
	outb(DMA_CLEAR_FF, 0xFF);

	// Step 5: Set count
	outb(DMA_COUNT_CH2, 0xFF);
	outb(DMA_COUNT_CH2, 0x01);

	// Step 6: Set page
	outb(DMA_PAGE_PORT, page);

	// Step 7: Set mode (single transfer, read, channel 2)
	outb(DMA_MODE_REG, 0x56);  // 0b01 000 110: single, address increment, read, channel 2

	// Step 8: Unmask channel 2
	outb(DMA_MASK_REG, 0x02);
}

void fdc_init()
{
	dma_floppy_prepare_read(DMA_BUFFER_ADDR, 1024);
	trap_set_gate(TRAP_IRQ(6), floppy_irq_handler);
}

static void floppy_sense_interrupt(uint8_t *st0, uint8_t *cyl)
{
	floppy_send_command(FLOPPY_CMD_SENSE_INT);
	*st0 = floppy_read_data();
	*cyl = floppy_read_data();
	printfd("FLOPPY: [sense_interrupt] st0 = %x, cyl = %x\n", *st0, *cyl);
}

static void floppy_recalibrate(int drive)
{
	printfd("FLOPPY: [recalibrate]\n");
	floppy_send_command_arg(FLOPPY_CMD_RECALIBRATE, drive);
	floppy_wait_irq();
	uint8_t st0, cyl;
	floppy_sense_interrupt(&st0, &cyl);
	if (cyl != 0) {
		printfd("FLOPPY: Recalibrate failed, expected cylinder 0, got %d\n", cyl);
	}
}

int floppy_seek(int drive, uint8_t cylinder, uint8_t head)
{
	printfd("FLOPPY: [seek] drive = %d, cyl = %d, head = %d\n", drive,
		cylinder, head);
	floppy_send_command(FLOPPY_CMD_SEEK);
	floppy_send_command((head << 2) | drive);
	floppy_send_command(cylinder);

	floppy_wait_irq();

	uint8_t st0, cyl;
	floppy_sense_interrupt(&st0, &cyl);
	if (cyl != cylinder) {
		printfd("FLOPPY: Seek failed: expected cyl %d, got %d\n", cylinder, cyl);
		return -1;
	}

	return 0;
}

void floppy_motor_on(int drive)
{
	outb(DIGITAL_OUTPUT_REGISTER, 0x1C | drive);
}

int fdc_read_sector(int drive, struct CHS *chs, int sector_count, uint8_t *buffer)
{
	floppy_recalibrate(drive);

	printfd("FLOPPY: Starting read_sector: drive=%d C/H/S=%d/%d/%d\n",
		drive, chs->cylinder, chs->head, chs->sector);
		
	floppy_motor_on(drive);
	
	if (floppy_seek(drive, chs->cylinder, chs->head) != 0) {
		printfd("FLOPPY: Seek failed\n");
		return -1;
	}
	
	floppy_send_command(FLOPPY_CMD_READ);
	floppy_send_command((chs->head << 2) | drive);
	floppy_send_command(chs->cylinder);
	floppy_send_command(chs->head);
	floppy_send_command(chs->sector);
	floppy_send_command(2);
	floppy_send_command(FLOPPY_SECTORS_PER_TRACK);
	floppy_send_command(0x1B);
	floppy_send_command(0xFF);
	
	printfd("FLOPPY: Will wait for IRQ\n");
	floppy_wait_irq();
	
	uint8_t st[7];
	for (int i = 0; i < 7; i++) {
		st[i] = floppy_read_data();
	}
	
	uint8_t ST0 = st[0];
	uint8_t ST1 = st[1];
	uint8_t ST2 = st[2];
	uint8_t cylinder = st[3];
	uint8_t head = st[4];
	uint8_t sector = st[5];
	uint8_t sector_size = st[6];

	(void) ST1; (void) ST2; (void) cylinder; (void) head; (void) sector;

	printfd("FLOPPY: st0=%x,st1=%x,st2=%x,cyl=%x,head=%x,sect=%x,sect_size=%x\n",
		ST0, ST1, ST2, cylinder, head, sector, sector_size);
	
	memcpy(buffer, (void*)DMA_BUFFER_ADDR, FLOPPY_SECTOR_SIZE * sector_count);
	
	if ((ST0 & 0xC0) != 0) {
		printfd("FLOPPY: Error: ST0 indicates error (ST0 = %x)\n", ST0);
		return -1;
	}	
	if (sector_size != 2) {
		printfd("FLOPPY: Error: Unexpected sector size code (got %d, expected 2)\n", sector_size);
		return -1;
	}

	return 0;
}

int fdc_write_sector(int drive, struct CHS *chs, int sector_count, uint8_t *buffer)
{
	outb(DIGITAL_OUTPUT_REGISTER, 0x1C | drive); // turn motor on

	floppy_send_command(FLOPPY_CMD_WRITE);
	floppy_send_command((chs->head << 2) | drive);
	floppy_send_command(chs->cylinder);
	floppy_send_command(chs->head);
	floppy_send_command(chs->sector);
	floppy_send_command(2); // 512 bytes
	floppy_send_command(FLOPPY_SECTORS_PER_TRACK);
	floppy_send_command(0x1B); // GAP3 length
	floppy_send_command(0xFF); // data length (unused)

	for (int i = 0; i < FLOPPY_SECTOR_SIZE * sector_count; i++) {
		floppy_write_data(buffer[i]);
	}

	floppy_wait_irq();

	for (int i = 0; i < 7; i++) {
		floppy_read_data();
	}

	return 0;
}
