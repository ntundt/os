#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct fat16_boot_sector{
	uint8_t jmp[3];
	uint8_t oem[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t number_of_fats;
	uint16_t root_entries;
	uint16_t total_sectors;
	uint8_t media_descriptor;
	uint16_t sectors_per_fat;
	uint16_t sectors_per_track;
	uint16_t number_of_heads;
	uint32_t hidden_sectors;
	uint32_t large_sectors;
	uint8_t drive_number;
	uint8_t current_head;
	uint8_t boot_signature;
	uint32_t volume_id;
	uint8_t volume_label[11];
	uint8_t system_id[8];
	uint8_t boot_code[448];
	uint16_t boot_sector_signature;
} __attribute__((packed));

enum FileSystem 
{
	UNKNOWN,
	FAT12,
	FAT16,
	FAT32,
	EXT2,
	EXT3,
	EXT4,
	NTFS
};

struct Parameters 
{
	const char *image;
	const char *bootloader;
	enum FileSystem fs;
};

void scan_fs(char *argv, struct Parameters *params)
{
	if (strcmp(argv, "fat12") == 0) {
		params->fs = FAT12;
	} else if (strcmp(argv, "fat16") == 0) {
		params->fs = FAT16;
	} else if (strcmp(argv, "fat32") == 0) {
		params->fs = FAT32;
	} else if (strcmp(argv, "ext2") == 0) {
		params->fs = EXT2;
	} else if (strcmp(argv, "ext3") == 0) {
		params->fs = EXT3;
	} else if (strcmp(argv, "ext4") == 0) {
		params->fs = EXT4;
	} else if (strcmp(argv, "ntfs") == 0) {
		params->fs = NTFS;
	} else {
		params->fs = UNKNOWN;
	}
}

void scan_parameters(int argc, char **argv, struct Parameters *params)
{
	for (int i = 0; i < argc - 1; i++) {
		if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--image") == 0) {
			params->image = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-b") == 0 
			|| strcmp(argv[i], "--bootloader") == 0) {
			params->bootloader = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fs") == 0) {
			scan_fs(argv[i + 1], params);
			i++;
		}
	}
}

void check_parameters_set(struct Parameters *params)
{
	if (params->image == NULL) {
		perror("Image file not set.");
		exit(1);
	} else if (params->bootloader == NULL) {
		perror("Bootloader file not set.");
		exit(1);
	} else if (params->fs == UNKNOWN) {
		perror("File system not set.");
		exit(1);
	}
}

void write_bootloader_fat12(FILE *image, FILE *bootloader)
{
	uint8_t buffer[512];

	struct fat16_boot_sector *boot_sector = (struct fat16_boot_sector *)buffer;

	// read bootloader
	size_t read = fread(buffer, 1, 512, bootloader);
	if (read != 512) {
		perror("Bootloader file does not match the size of 512 bytes.");
		exit(1);
	}

	// jmp instruction
	fseek(image, 0, SEEK_SET);
	fwrite(&(boot_sector->jmp), 3, 1, image);

	// boot code
	fseek(image, (int)((void *)&(boot_sector->boot_code) - (void *)boot_sector), SEEK_SET);
	fwrite(&(boot_sector->boot_code), 1, sizeof(boot_sector->boot_code), image);
}

void write_bootloader(struct Parameters *params)
{
	int err = 0;

	FILE *image = fopen(params->image, "r+");
	FILE *bootloader = fopen(params->bootloader, "r");

	if (image == NULL) {
		perror("Image file not found.");
		exit(1);
	} else if (bootloader == NULL) {
		perror("Bootloader file not found.");
		exit(1);
	}

	switch (params->fs) {
	case FAT12:
		write_bootloader_fat12(image, bootloader);
		break;
	case FAT16:
	case FAT32:
	case EXT2:
	case EXT3:
	case EXT4:
	case NTFS:
	case UNKNOWN:
		perror("Not implemented yet for this file system.");
		err = 1;
	}

	fclose(image);
	fclose(bootloader);

	if (err) {
		exit(1);
	}
}

int main(int argc, char **argv) 
{
	struct Parameters params;
	scan_parameters(argc, argv, &params);
	check_parameters_set(&params);
	write_bootloader(&params);
	printf("Bootloader written successfully.\n");
	return 0;
}
