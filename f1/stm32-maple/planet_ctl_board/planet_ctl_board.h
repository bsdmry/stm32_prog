#define TONE_PORT GPIOB
#define TONE_PIN GPIO5
#define TONE_PRESCALER 20 //20 - 500 Hz

#define LED_REFRESH_PRESCALER 100
#define LED_BLINK_PRESCALER 1000
#define KEYBOARD_POLLER_PRESCALER 1600
#define KEY_POLLER_PRESCALER 1300

#define LED_PORT GPIOB
#define RX_LED_PIN GPIO1
#define TX_LED_PIN GPIO0


#define RELAY_PORT GPIOA
#define RELAY_PIN GPIO7

#define BTN0_PORT GPIOB
#define BTN0_PIN GPIO12
#define BTN1_PORT GPIOA
#define BTN1_PIN GPIO10

#define KEY_PORT GPIOB
#define KEY_DAH_PIN GPIO6
#define KEY_DIT_PIN GPIO7

void tim3_init(void);
void tim2_init(void);
void display_setup(void);
void sound_setup(void);
void controls_setup(void);
void rf_setup(void);
void btn0_action(void);
void btn1_action(void);
void rotary_action(void);
void radio_init_cfg(void);
void set_encoder(uint32_t value, uint32_t max_value);

void enable_tx(void);
void disable_tx(void);

void setup_cw_len(uint8_t signal);
void display_frequency(void);
void update_frequency(uint32_t value);
void audio_switch2tx(void);
void audio_switch2rx(void);

void rf_switch2rx(void);
void rf_switch2tx(void);

void show_main_screen(void);
void show_menu_rx_audio_vol(void);
void show_menu_tx_tone_vol(void); 
void show_menu_tx_tone_freq(void);
void show_menu_tx_offset(void); 
void show_menu_step_select(void); 
void show_menu_cw_speed(void);

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
	uint8_t cwSpeeed;
	uint16_t txOffset;
	uint8_t currentMenuEntity;
	uint8_t stepSize;
	uint8_t freqDisplayMode;
} planet;
