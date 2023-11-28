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
	uint16_t page_size;
	uint8_t options_count;
	option_record* options;
} flash_storage;

flash_storage* flash_memory_storage_init(uint32_t flash_memory_end_address, uint8_t page_count, uint16_t page_size);

uint32_t flash_memory_storage_make_record_word(uint8_t record_id, uint16_t len, uint32_t address);
void flash_memory_storage_decode_record_word(option_record* record, uint32_t word);

void flash_memory_config_container_save(flash_storage* storage);
void flash_memory_config_container_restore(flash_storage* storage);

void flash_memory_write_option(flash_storage* storage, uint8_t id, uint16_t data_len, uint32_t *data);
void flash_memory_read_option(flash_storage* storage, uint8_t id, uint32_t *data);

void flash_memory_storage_read(uint32_t address, uint32_t *read_data);
void flash_memory_storage_write(uint32_t address, uint32_t write_data);
uint32_t flash_memory_storage_erase(flash_storage *fstore);
#endif
