#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "dwt.h"
#include "usb_cdc.h"
#include "prog_eeprom.h"

// PA0-PA7 - data
// PB12 - ADDR0
// PB13 - ADDR1
// PB14 - ADDR2
// PB15 - ADDR3
// PA8 - ADDR4
// PA9 - ADDR5
// PA10 - ADDR6
// PA15 - ADDR7
// PB3 - ADDR8
// PB4 - ADDR9
// PB5 - ADDR10
// PB6 - ADDR11
// PB7 - ADDR12
// PB8 - ADDR13
// PB9 - ADDR14
// PB10 - ADDR15

//READ - PB1
//WRITE -PB0

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

#define READ GPIO1
#define WRITE GPIO0

#define MODE_READ 0
#define MODE_WRITE 1

uint32_t const addr_port[16] = {
								GPIOB, GPIOB, GPIOB, GPIOB,
								GPIOA, GPIOA, GPIOA, GPIOA,
								GPIOB, GPIOB, GPIOB, GPIOB,
								GPIOB, GPIOB, GPIOB, GPIOB
								};
uint16_t const addr_pin[16] = {
								ADDR0, ADDR1, ADDR2, ADDR3,
								ADDR4, ADDR5, ADDR6, ADDR7,
								ADDR8, ADDR9, ADDR10, ADDR11,
								ADDR12, ADDR13, ADDR14, ADDR15,	
								};
uint16_t const data_pin[8] = {
								DATA0, DATA1, DATA2, DATA3,
								DATA4, DATA5, DATA6, DATA7
								};

uint8_t data_bus_mode = MODE_WRITE;
uint8_t address_bus_mode = MODE_WRITE;
uint8_t rw_pins_mode = MODE_WRITE;
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
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, DATA0 | DATA1 | DATA2 | DATA3 | DATA4 | DATA5 | DATA6 | DATA7);
}
void set_data_bus_write_mode(void){
	data_bus_mode = MODE_WRITE;
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DATA0 | DATA1 | DATA2 | DATA3 | DATA4 | DATA5 | DATA6 | DATA7);
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
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, READ | WRITE); 
}

void set_rw_pins_read_mode(void){
	rw_pins_mode = MODE_READ;
	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, READ | WRITE); 
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
			gpio_set(GPIOA, data_pin[i]);
		} else {
			gpio_clear(GPIOA, data_pin[i]);
		}
	}	
}

uint8_t get_data(void){
	if (data_bus_mode != MODE_READ){
		set_data_bus_read_mode();
	}
	uint8_t val = 0;
	for (uint8_t i = 0; i < 7; i++){
		if (gpio_get(GPIOA, data_pin[i]) != 0x00){
			val |= (0x01 << i);
		} else {
			val &= ~(0x01 << i);
		}
	}
	return val;
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
	if (inputcmd[0] == 0x57){ //'W' - write
		uint16_t address = (((uint16_t)inputcmd[1] << 8) | (uint16_t)inputcmd[2]);
		uint8_t data = (uint8_t)inputcmd[3];
		set_address(address);
		set_data(data);
		cdc_send(&reply[0], 5);
	} else if {

	}
}

int main(void)
{
	clock_setup();
	dwt_setup();
	init_usb_cdc(0);
	tim3_setup();
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
	gpio_clear(GPIOC, GPIO13);
	uint8_t dat = 0xAA;
	uint16_t add = 0xDEAD;
	char output[20] = "A: 0xFFFF D: OxFF \r\n"; //"A: 0xFFFF D: OxFF\r\n"
	char buflen[11] = "BUF 0x00 \r\n";
	val2hexstr((uint32_t)add, &output[5], 4);
	val2hexstr((uint32_t)dat, &output[15], 2);
	set_data_bus_write_mode();
	set_address_bus_write_mode();
	set_rw_pins_read_mode();
	

	while(1){
		cdcacm_data_tx(usbd_dev_g);
		if(rx_ring_buffer.length > 0){
			//gpio_toggle(GPIOC, GPIO13);

		//	val2hexstr((uint32_t)rx_ring_buffer.length, &buflen[6], 2);
		//	cdc_send(&buflen[0], 11);
			//cdcacm_data_tx(usbd_dev_g);

			for (uint8_t i = 0; i < MAX_PACKET_SIZE; i++){
					uint8_t res = rb_u8_pop(&rx_ring_buffer, &inputcmd[i]);
					if (inputcmd[i] == 0x0A) {
						parse_cmd();
					}
					if (res == 0){
						break;
					}
			}			
		}
		//dwt_delay_ms(1000);
		//dat = get_data();
		//add = get_address();
		//val2hexstr((uint32_t)add, &output[5], 4);
		//set_data(dat);
		//val2hexstr((uint32_t)dat, &output[15], 2);
		//cdc_print(&output[0]);
		//cdcacm_data_tx(usbd_dev_g);
		
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

