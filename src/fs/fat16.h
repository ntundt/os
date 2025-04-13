#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>

#define FAT16_DIRENTRY_FLAG_READONLY 0x01
#define FAT16_DIRENTRY_FLAG_HIDDEN 0x02
#define FAT16_DIRENTRY_FLAG_SYSTEM 0x04
#define FAT16_DIRENTRY_FLAG_VOLUME_NAME 0x08
#define FAT16_DIRENTRY_FLAG_DIRECTORY 0x10

struct fat16_boot_sector {
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

struct fat16_direntry {
	uint8_t name[11];
	uint8_t attributes;
	uint8_t nt_reserved;
	uint8_t created_tenths;
	uint16_t created_time;
	uint16_t created_date;
	uint16_t accessed_date;
	uint16_t fat32_reserved;
	uint16_t writed_time;
	uint16_t writed_date;
	uint16_t first_cluster_low;
	uint32_t file_size_bytes;
} __attribute__((packed));

#endif
