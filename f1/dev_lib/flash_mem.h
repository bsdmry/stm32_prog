#ifndef _H_FLASH_STORAGE
#define _H_FLASH_STORAGE
#include <stdlib.h>
#include <libopencm3/stm32/flash.h>
#define FLASH_PAGE_SIZE_MEDIUM ((uint16_t)0x400) //1024 bytes for medium density devices (RM008)
#define FLASH_MIN_ADDRES ((uint32_t)0x08000000)
#define FLASH_MAX_ADDRES_F103C8 ((uint32_t)0x0800FFFF) //64 Kb
#define FLASH_LAST_PAGE_ADDR_F103C8 (FLASH_MAX_ADDRES_F103C8 - FLASH_PAGE_SIZE_MEDIUM)


typedef struct {
	uint8_t record_id;
	uint16_t len;
	uint32_t address;
} option_record;

typedef struct {
	uint32_t flash_end_address;
	uint32_t storage_start_address;
	uint8_t page_count;
	uint8_t page_size;
	uint8_t records_count;
	uint32_t *records_metadata;
} flash_storage;
flash_storage* flash_memory_storage_init(uint32_t flash_memory_end_address, uint8_t page_count, uint16_t page_size);
uint32_t flash_memory_storage_make_option_record(uint8_t record_id, uint16_t len, uint32_t address);
void flash_memory_storage_read(uint32_t address, uint32_t *read_data);
void flash_memory_storage_write(uint32_t address, uint32_t write_data);
uint32_t flash_memory_storage_erase(flash_storage *fstore);
#endif
