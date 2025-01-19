
void set_data_bus_read_mode(void);
void set_data_bus_write_mode(void);
void set_address_bus_read_mode(void);
void set_address_bus_write_mode(void);
void set_rw_pins_write_mode(void);
void set_rw_pins_read_mode(void);
uint32_t hexstr2val(char *str, uint8_t len);
void val2hexstr(uint32_t val, char* str, uint8_t len);
void set_address(uint16_t address);
uint16_t get_address(void);
void set_data(uint8_t data);
uint8_t get_data(void);
void parse_cmd(void);
void set_rw(uint8_t rw_mode);
uint8_t get_rw(void);
void eeprom_write(uint16_t address, uint8_t data);
uint8_t eeprom_read(uint16_t address);


