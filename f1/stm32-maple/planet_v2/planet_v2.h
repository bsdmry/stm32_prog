#define BTN0_PORT GPIOA
#define BTN0_PIN GPIO10 //Encoder button
#define BTN1_PORT GPIOB
#define BTN1_PIN GPIO14 //Left button
#define BTN2_PORT GPIOB
#define BTN2_PIN GPIO5 //Right button

#define KEY_PORT GPIOB
#define KEY_DAH_PIN GPIO6
#define KEY_DIT_PIN GPIO7

#define KEYBOARD_POLLER_PRESCALER 1600
#define KEY_POLLER_PRESCALER 1300

#define TONE_PORT GPIOC
#define TONE_PIN GPIO13
#define TONE_PRESCALER 20 //20 - 500 Hz

#define SND_MIX_PORT GPIOC
#define SND_MIX_CLK_PIN GPIO15
#define SND_MIX_DATA_PIN GPIO14

#define RELAY_PORT GPIOA
#define RELAY_PIN GPIO7

#define LED_PORT GPIOB
#define RX_LED_PIN GPIO1
#define TX_LED_PIN GPIO0

void tim3_init(void);
void tim2_init(void);
void sound_setup(void);
void controls_setup(void);
void display_setup(void);
void rf_setup(void);
void rotary_action(void);
void adc_setup(void);

void radio_init_cfg(void);
void show_main_screen(void);
void setup_cw_len(uint8_t signal);
void display_frequency(void);
void update_frequency(uint32_t value);
void set_encoder(uint32_t value, uint32_t max_value);
uint32_t read_adc(void);

void btn0_action(void); //chg vol
void btn1_action(void); //config-enter
void btn2_action(void);//exit-chg freq view

void enable_tx(void);
void disable_tx(void);

void audio_switch2tx(void);
void audio_switch2rx(void);

void rf_switch2rx(void);
void rf_switch2tx(void);

void show_menu_rx_audio_vol(void);
void show_menu_tx_tone_vol(void); 
void show_menu_tx_tone_freq(void);
void show_menu_tx_offset(void); 
void show_menu_step_select(void); 
void show_menu_cw_speed(void);

void redisplay_rx_audio_vol(void);
void redisplay_tx_tone_vol(void);
void redisplay_tx_tone_freq(void);
void redisplay_tx_offset(void);
void redisplay_step(void);
void redisplay_cw_speed(void);

void show_edit_rx_audio_vol(void); 
void show_edit_tx_tone_vol(void);
void show_edit_tx_tone_freq(void); 
void show_edit_tx_offset(void);
void show_edit_step(void);
void show_edit_cw_speed(void);

void show_update_rx_audio_vol(uint32_t value);
void show_update_tx_tone_vol(uint32_t value);
void show_update_tx_tone_freq(uint32_t value);
void show_update_tx_offset(uint32_t value);
void show_update_step(uint32_t value);
void show_update_cw_speed(uint32_t value);

void roll_over_menu(uint16_t position);

typedef struct {
	uint32_t frequency;
	uint16_t kHz;
	uint16_t hHz;
	uint16_t rxVolLevel;
	uint16_t toneVolLevel;
	uint16_t toneFreq;
	uint8_t cwSpeed;
	uint16_t txOffset;
	uint8_t currentMenuEntity;
	uint8_t stepSize;
} planet_v2;
