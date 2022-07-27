#define MAX_FREQ 1300000000L
#define MIN_FREQ 500000
void set_frequency(int32_t f);
int32_t get_frequency(void);
void inc_frequency(int32_t step);
void dec_frequency(int32_t step);
void set_volume(int16_t v);
int16_t get_volume(void);
void inc_volume(int16_t step);
void dec_volume(int16_t step);
void set_squelch(int16_t s);
int16_t get_squelch(void);
void inc_squelch(int16_t step);
void dec_squelch(int16_t step);

