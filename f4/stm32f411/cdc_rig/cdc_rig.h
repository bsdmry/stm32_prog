#include "ic746_ctl.h"

typedef struct RigState {
	uint8_t ptt;
	long freqA;
	long freqB;
	uint8_t mode;
	uint8_t smeter;
	uint8_t vfo;
} RigState;

void set_ptt(uint8_t state);
uint8_t get_ptt(void);
void set_freq(long f);
long get_freq(void);
void set_mode(uint8_t state);
uint8_t get_mode(void);
void set_split(uint8_t state);
void AtoB(void);
void swap_vfo(void);
void set_vfo(uint8_t state);
uint8_t get_smeter(void);

void bind_handlers(void);
