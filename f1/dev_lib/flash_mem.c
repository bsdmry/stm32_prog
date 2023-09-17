#include "flash_mem.h"

flash_storage* flash_memory_storage_init(uint32_t flash_memory_end_address, uint8_t page_count, uint16_t page_size){
	flash_storage* storage = malloc(sizeof(flash_storage));
	storage->storage_start_address =  (flash_memory_end_address - (uint32_t)(page_count* page_size));
	storage->flash_end_address = flash_memory_end_address;
	storage->page_count = page_count;
	storage->page_size = page_size;
	/*uint32_t num_option_records= 0;
	flash_memory_storage_read(storage->storage_start_address, &num_option_records);
	if (flash_memory_storage_read){
		storage->records_count = (uint8_t)num_option_records;
		storage->records_metadata = malloc(sizeof(uint32_t) * num_option_records)	
	}*/
	return storage;
}

uint32_t flash_memory_storage_make_option_record(uint8_t record_id, uint16_t len, uint32_t address){
	uint32_t record = 0x00;
	record = ((uint32_t)(record_id & 0x3F)) << 10; //6 bit id
	record = record | ((uint32_t)(len & 0x03FF) << 15); //10 bit record size
	record = record | (address - FLASH_MIN_ADDRES); //calculate offset (16 bit)
	return record;
}

void flash_memory_storage_read(uint32_t address, uint32_t *read_data){
	uint32_t *ptr= (uint32_t*)address;
	*(uint32_t*)read_data = *ptr;
}
void flash_memory_storage_write(uint32_t address, uint32_t write_data){
	flash_unlock();
	flash_program_word(address, write_data);
	flash_lock();
}
/*
uint32_t flash_memory_storage_read_option(uint8_t record_id){
	uint16_t value = 0;
       	
}*/

uint32_t flash_memory_storage_erase(flash_storage *fstore){
	uint32_t flash_status = 0;
	flash_unlock();
	for (uint8_t i = fstore->page_count; i > 0; i--){
		uint32_t ed = (fstore->flash_end_address - (uint32_t)(i* fstore->page_size));
		flash_erase_page(ed);
		flash_status = flash_get_status_flags();
		if(flash_status != FLASH_SR_EOP){
			flash_lock();
			return flash_status;
		}
	}
	flash_lock();
	return flash_status;
}
