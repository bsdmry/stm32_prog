#include "flash_mem.h"

flash_storage* flash_memory_storage_init(uint32_t flash_memory_end_address, uint8_t page_count, uint16_t page_size){
	flash_storage* storage = malloc(sizeof(flash_storage));
	storage->storage_start_address =  (flash_memory_end_address - (uint32_t)(page_count* page_size)) + 1;
	storage->flash_end_address = flash_memory_end_address;
	storage->page_count = page_count;
	storage->page_size = page_size;
	return storage;
}

uint32_t flash_memory_storage_make_record_word(uint8_t record_id, uint16_t len, uint32_t address){
	uint32_t record = 0x00;
	record = ((uint32_t)(record_id & 0x3F)) << 26; //6 bit id
	record = record | ((uint32_t)(len & 0x03FF) << 16); //10 bit record size
	record = record | (address - FLASH_MIN_ADDRES); //calculate offset (16 bit)
	return record;
}
void flash_memory_storage_decode_record_word(option_record* record, uint32_t word){
	record->record_id = (uint8_t)(word >> 26);
	record->len = (uint16_t)((word & 0x03FF0000) >> 16);
	record->address = (word & 0x0000FFFF) + FLASH_MIN_ADDRES;
}

void flash_memory_config_container_save(flash_storage* storage){
	uint32_t index_start_address = storage->storage_start_address + sizeof(uint32_t);
	uint32_t data_start_address = index_start_address + (sizeof(uint32_t))*storage->options_count;
	uint32_t index_record_word = 0;
	flash_memory_storage_write(storage->storage_start_address, (uint32_t)storage->options_count); //Save number of option records
	for (uint8_t i = 0; i < storage->options_count; i++){
		index_record_word = flash_memory_storage_make_record_word(storage->options[i].record_id, storage->options[i].len, data_start_address);
		storage->options[i].address = data_start_address;
		flash_memory_storage_write(index_start_address, index_record_word);
		index_start_address = index_start_address + sizeof(uint32_t);
		data_start_address = data_start_address + (sizeof(uint32_t))*storage->options[i].len;
	}
}

void flash_memory_config_container_restore(flash_storage* storage){
	uint32_t record_nums = 0;
	uint32_t index_start_address = storage->storage_start_address + sizeof(uint32_t);
	flash_memory_storage_read(storage->storage_start_address, &record_nums); //Restore number of options
	storage->options_count = (uint8_t)record_nums;
	storage->options = malloc(sizeof(option_record)* record_nums);

	for (uint8_t i = 0; i < storage->options_count; i++){
		uint32_t record_word = 0;
		flash_memory_storage_read(index_start_address, &record_word);
		flash_memory_storage_decode_record_word(&(storage->options[i]), record_word);
		index_start_address = index_start_address + sizeof(uint32_t);
	}
}
void flash_memory_write_option(flash_storage* storage, uint8_t id, uint16_t data_len, uint32_t *data){
	if (data_len > 1023){
		data_len = 1023;
	}
	if (id > 63) {
		id = 63;
	}
	for (uint8_t i = 0; i < storage->options_count; i++){
		if (storage->options[i].record_id == id){
			flash_unlock();
			for (uint16_t k=0; k< data_len; k++){
				flash_program_word((storage->options[i].address + k*sizeof(uint32_t)), data[k]);
			}
			flash_lock();
			break;
		}
	}
}

void flash_memory_read_option(flash_storage* storage, uint8_t id, uint32_t *data){
	for (uint8_t i = 0; i < storage->options_count; i++){
		if (storage->options[i].record_id == id){
			for (uint16_t k=0; k< storage->options[i].len; k++){
				flash_memory_storage_read((storage->options[i].address + k*sizeof(uint32_t)), &data[k]);
			}
			break;
		}
	}
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

uint32_t flash_memory_storage_erase(flash_storage *fstore){
	uint32_t flash_status = 0;
	flash_unlock();
	for (uint8_t i = fstore->page_count; i > 0; i--){
		//uint32_t ed = (fstore->flash_end_address - (uint32_t)(i* fstore->page_size));
		flash_erase_page(fstore->storage_start_address);
		flash_status = flash_get_status_flags();
		if(flash_status != FLASH_SR_EOP){
			flash_lock();
			return flash_status;
		}
	}
	flash_lock();
	return flash_status;
}
