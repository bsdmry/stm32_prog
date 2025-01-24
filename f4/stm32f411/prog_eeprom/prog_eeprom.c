#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "dwt.h"
#include "usb_cdc.h"
#include "prog_eeprom.h"

#define DATA_PORT GPIOA
#define DATA0 GPIO0
#define DATA1 GPIO1
#define DATA2 GPIO2
#define DATA3 GPIO3
#define DATA4 GPIO4
#define DATA5 GPIO5
#define DATA6 GPIO6
#define DATA7 GPIO7

#define ADDR0 GPIO12
#define ADDR1 GPIO13
#define ADDR2 GPIO14
#define ADDR3 GPIO15
#define ADDR4 GPIO8
#define ADDR5 GPIO9
#define ADDR6 GPIO10
#define ADDR7 GPIO15
#define ADDR8 GPIO3
#define ADDR9 GPIO4
#define ADDR10 GPIO5
#define ADDR11 GPIO6
#define ADDR12 GPIO7
#define ADDR13 GPIO8
#define ADDR14 GPIO9
#define ADDR15 GPIO10

#define RW_PORT GPIOB
#define READ GPIO1
#define WRITE GPIO0

#define OE_PORT GPIOC
#define OE GPIO15

#define CE_PORT GPIOC
#define CE GPIO14

#define MODE_READ 0
#define MODE_WRITE 1

#define PIN_LOW 0
#define PIN_HIGH 1

#define RW_CMD_READ 0
#define RW_CMD_WRITE 1
#define RW_CMD_IDLE 2

uint32_t const addr_port[16] = {
								GPIOB, GPIOB, GPIOB, GPIOB,			// GPIO12, GPIO13, GPIO14, GPIO15
								GPIOA, GPIOA, GPIOA, GPIOA,			// GPIO8,  GPIO9,  GPIO10, GPIO15
								GPIOB, GPIOB, GPIOB, GPIOB,			// GPIO3,  GPIO4,  GPIO5,  GPIO6,
								GPIOB, GPIOB, GPIOB, GPIOB			// GPIO7,  GPIO8,  GPIO9,  GPIO10
								};
uint16_t const addr_pin[16] = {
								ADDR0, ADDR1, ADDR2, ADDR3,			// GPIO12, GPIO13, GPIO14, GPIO15
								ADDR4, ADDR5, ADDR6, ADDR7,			// GPIO8,  GPIO9,  GPIO10, GPIO15
								ADDR8, ADDR9, ADDR10, ADDR11,		// GPIO3,  GPIO4,  GPIO5,  GPIO6,
								ADDR12, ADDR13, ADDR14, ADDR15,		// GPIO7,  GPIO8,  GPIO9,  GPIO10
								};
uint16_t const data_pin[8] = {
								DATA0, DATA1, DATA2, DATA3,			// GPIO0,  GPIO1,  GPIO2,  GPIO3,
								DATA4, DATA5, DATA6, DATA7			// GPIO4,  GPIO5,  GPIO6,  GPIO7
								};

uint8_t data_bus_mode = MODE_READ;
uint8_t address_bus_mode = MODE_READ;
uint8_t rw_pins_mode = MODE_READ;
volatile uint8_t buf_show = 0;	
char inputcmd[5];

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_TIM3);
}

static void tim3_setup(void)
  {
      rcc_periph_reset_pulse(RST_TIM3);
      timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
      uint32_t prescaler = 62499;
      uint32_t period = 1535;
  
      timer_continuous_mode(TIM3);
      timer_set_prescaler(TIM3, prescaler -1 );
      timer_set_period(TIM3, period -1 );
      timer_set_counter(TIM3,0);
      /* Counter enable. */
      timer_enable_counter(TIM3);
      timer_enable_irq(TIM3, TIM_DIER_UIE);
      nvic_enable_irq(NVIC_TIM3_IRQ);
  
  
  }


void set_data_bus_read_mode(void){
	data_bus_mode = MODE_READ;
	gpio_mode_setup(DATA_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, DATA0 | DATA1 | DATA2 | DATA3 | DATA4 | DATA5 | DATA6 | DATA7);
}
void set_data_bus_write_mode(void){
	data_bus_mode = MODE_WRITE;
	gpio_mode_setup(DATA_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DATA0 | DATA1 | DATA2 | DATA3 | DATA4 | DATA5 | DATA6 | DATA7);
}

void set_address_bus_read_mode(void){
	address_bus_mode = MODE_READ;
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ADDR4 | ADDR5 | ADDR6 | ADDR7);
	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ADDR0 | ADDR1 | ADDR2 | ADDR3 | ADDR8 | ADDR9 | ADDR10 | ADDR11 | ADDR12 | ADDR13 | ADDR14 | ADDR15);
}

void set_address_bus_write_mode(void){
	address_bus_mode = MODE_WRITE;
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ADDR4 | ADDR5 | ADDR6 | ADDR7);
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ADDR0 | ADDR1 | ADDR2 | ADDR3 | ADDR8 | ADDR9 | ADDR10 | ADDR11 | ADDR12 | ADDR13 | ADDR14 | ADDR15);
}

void set_rw_pins_write_mode(void){
	rw_pins_mode = MODE_WRITE;
	gpio_mode_setup(RW_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, READ | WRITE); 
}

void set_rw_pins_read_mode(void){
	rw_pins_mode = MODE_READ;
	gpio_mode_setup(RW_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, READ | WRITE); 
}

void set_oe_pin_write_mode(void){
	gpio_mode_setup(OE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, OE); 
}

void set_ce_pin_write_mode(void){
	gpio_mode_setup(CE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CE); 
}


void set_address(uint16_t address){
	if (address_bus_mode != MODE_WRITE){
		set_address_bus_write_mode();
	}
	for (uint8_t i = 0; i < 16; i++){
		if (address & (0x01 << i)){
			gpio_set(addr_port[i], addr_pin[i]);
		} else {
			gpio_clear(addr_port[i], addr_pin[i]);
		}
	}	
}

uint16_t get_address(void){
	if (address_bus_mode != MODE_READ){
		set_address_bus_read_mode();
	}
	uint16_t val = 0;
	for (uint8_t i = 0; i < 16; i++){
		if (gpio_get(addr_port[i], addr_pin[i]) != 0x00){
			val |= (0x01 << i);
		} else {
			val &= ~(0x01 << i);
		}
	}
	return val;
}

void set_data(uint8_t data){
	if (data_bus_mode != MODE_WRITE){
		set_data_bus_write_mode();
	}
	for (uint8_t i = 0; i < 8; i++){
		if (data & (0x01 << i)){
			gpio_set(DATA_PORT, data_pin[i]);
		} else {
			gpio_clear(DATA_PORT, data_pin[i]);
		}
	}	
}


uint8_t get_data(void){
	if (data_bus_mode != MODE_READ){
		set_data_bus_read_mode();
	}
	uint8_t val = 0;
	for (uint8_t i = 0; i < 8; i++){
		if (gpio_get(DATA_PORT, data_pin[i]) != 0x00){
			val |= (0x01 << i);
		} else {
			val &= ~(0x01 << i);
		}
	}
	return val;
}

void set_rw(uint8_t rw_mode){
	if (rw_pins_mode != MODE_WRITE){
		set_rw_pins_write_mode();
	}
	if (rw_mode == RW_CMD_READ){ // Pin /RD low, active
		gpio_clear(RW_PORT, READ);
		gpio_set(RW_PORT, WRITE);
	} else if(rw_mode == RW_CMD_WRITE){ //Pin /WR low, active
		gpio_set(RW_PORT, READ);
		gpio_clear(RW_PORT, WRITE);
	} else { // Both are high - Idle
		gpio_set(RW_PORT, READ);
		gpio_set(RW_PORT, WRITE);
	}
}

uint8_t get_rw(void){
	if (rw_pins_mode != MODE_READ){
		set_rw_pins_read_mode();
	}	
	uint8_t status = 0x00;
	if (gpio_get(RW_PORT, WRITE) != 0x00){
		status = 0x01;
	}
	status = status << 1;
	if (gpio_get(RW_PORT, READ) != 0x00){
		status |= 0x01;
	}
	return status;
}

void set_oe(uint8_t rw_mode){
	if (rw_mode == PIN_HIGH){ // Pin /OE high,
		gpio_set(OE_PORT, OE);
	}else{
		gpio_clear(OE_PORT, OE); //All others - /OE low, active (read and idle)
	}		
}

void set_ce(uint8_t rw_mode){
	if (rw_mode == PIN_HIGH){ // Pin /CE high,
		gpio_set(CE_PORT, CE);
	}else{
		gpio_clear(CE_PORT, CE); //All others - /CE low, active (read and idle)
	}		
}

void eeprom_write(uint16_t address, uint8_t data){
	set_rw(RW_CMD_IDLE); //Write High, Read High

	set_oe(PIN_HIGH); //OE high
	set_address(address);
	set_ce(PIN_LOW);
	set_rw(RW_CMD_WRITE); // /WR low
	for(uint16_t i = 0; i < 100; i++){
		__asm("nop");
	}
	set_data(data);
	set_rw(RW_CMD_IDLE); // /WR high 
	set_ce(PIN_HIGH);
	set_oe(PIN_LOW);
}

uint8_t eeprom_read(uint16_t address){
	set_rw(RW_CMD_IDLE);
	uint8_t data = 0x00;
	set_address(address);
	set_rw(RW_CMD_READ);
	set_ce(PIN_LOW);
	set_oe(PIN_LOW);
	for(uint16_t i = 0; i < 100; i++){
		__asm("nop");
	}
	data = get_data();
	set_ce(PIN_HIGH);
	set_oe(PIN_HIGH);
	set_rw(RW_CMD_IDLE);
	return data;
}

void get_cdc_comm_config(uint32_t speed, uint8_t stop_bits, uint8_t parity, uint8_t data_bits){

}

uint32_t hexstr2val(char *str, uint8_t len){
    uint32_t r = 0;
    for(uint8_t i = 0; i < len; i++){
        uint8_t c = (uint8_t)str[i];
        uint8_t val = 0;
        if (c >= 0x30 && c <= 0x39){ //0-0
            val = c - 0x30;
        } else if (c >= 0x41 && c <= 0x46){ //A-F
            val = c - 0x37;
        } else { //a-f
            val = c- 0x57;
        }
        r = r << 4;
        r = r | val;
    }
    return r;
}

void val2hexstr(uint32_t val, char* str, uint8_t len){
    for (uint8_t i = len; i != 0; i--){
        uint8_t d = (uint8_t)(val & 0x0F);
        val = val >> 4;
        char c = '0';
        if (d > 9){
            c = (char)(d + 0x37);
        } else {
            c = (char)(d + 0x30);
        }
        str[i-1] = c;
    }
}

void parse_cmd(void){
	uint8_t reply[5] =  {0x3E, inputcmd[1], inputcmd[2], 0x01, 0x0A};
	uint16_t address = 0x0000;
	
	if (inputcmd[0] == 0x57){ //'W' - write
		address = (((uint16_t)inputcmd[1] << 8) | (uint16_t)inputcmd[2]);
		uint8_t data = (uint8_t)inputcmd[3];
		eeprom_write(address, data);
	} else if (inputcmd[0] == 0x52){ //'R'
		address = (((uint16_t)inputcmd[1] << 8) | (uint16_t)inputcmd[2]);
		reply[3] = eeprom_read(address);
	} else {
		reply[0] = 0x3F;  //send '?'
	}
	//char txt[] = ">0x0000 - 0x00\n";
	//val2hexstr((uint32_t)address, &txt[3], 4);
	//val2hexstr((uint32_t)reply[3], &txt[12], 2);
	//cdc_print(&txt[0]);
	cdc_send(&reply[0], 5);
}

int main(void)
{
	clock_setup();
	dwt_setup();
	init_usb_cdc(0);
	tim3_setup();
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
	gpio_clear(GPIOC, GPIO13);
	//val2hexstr((uint32_t)add, &output[5], 4);
	//val2hexstr((uint32_t)dat, &output[15], 2);
	set_data_bus_read_mode();
	set_address_bus_read_mode();
	set_rw(RW_CMD_IDLE);
	set_oe_pin_write_mode();
	set_ce_pin_write_mode();
	set_oe(PIN_HIGH);
	set_ce(PIN_HIGH);
	

	while(1){
		cdcacm_data_tx(usbd_dev_g);
		if(rx_ring_buffer.length > 0){
			for (uint8_t i = 0; i < MAX_PACKET_SIZE; i++){
					uint8_t res = rb_u8_pop(&rx_ring_buffer, &inputcmd[i]);
					if ((inputcmd[i] == 0x0A) && (i == 0x04)) {
						parse_cmd();
					}
					if (res == 0){
						break;
					}
			}			
		}
	};
}

void tim3_isr(void)
  {
      if (timer_interrupt_source(TIM3, TIM_SR_UIF)) {
			buf_show = 1;
          /* Clear compare interrupt flag. */
          timer_clear_flag(TIM3, TIM_SR_UIF);
      }
  }

