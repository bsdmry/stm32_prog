#include "si5351.h"
#include "i2c.h"

//LibOpenCM3 version of https://github.com/afiskon/stm32-si5351/blob/main/si5351/si5351.c
uint32_t si5351Correction;

void si5351_calculate_params(int32_t Fclk, si5351_pll_cfg* pll_conf, si5351_out_conf* out_conf) {
    if(Fclk < 1000000) {
        Fclk *= 64;
        out_conf->rdiv = SI5351_R_DIV_64;
    } else {
        out_conf->rdiv = SI5351_R_DIV_1;
    }

    // Apply correction, _after_ determining rdiv.
    Fclk = Fclk - ((Fclk/1000000)*si5351Correction)/100;

    const int32_t Fxtal = 25000000;
    int32_t a, b, c, x, y, z, t;
    if(Fclk < 81000000) {
        // Valid for Fclk in 0.5..112.5 Meg range
        // However an error is > 6 Hz above 81 Megs
        a = 36; // PLL runs @ 900 Meg
        b = 0;
        c = 1;
        int32_t Fpll = 900000000;
        x = Fpll/Fclk;
        // t = ceil(Fclk/0xFFFFF)
        t = Fclk >> 20;
        if(Fclk & 0xFFFFF) {
            t += 1;
        }
        y = (Fpll % Fclk) / t;
        z = Fclk / t;
    } else {
        // Valid for Fclk in 75..160 Meg range
        if(Fclk >= 150000000) {
            x = 4;
        } else if (Fclk >= 100000000) {
            x = 6;
        } else {
            x = 8;
        }
        y = 0;
        z = 1;

        int32_t numerator = x*Fclk;
        a = numerator/Fxtal;
        // t = ceil(Fxtal/0xFFFFF)
        t = Fxtal >> 20;
        if(Fxtal & 0xFFFFF) {
            t += 1;
        }
        b = (numerator % Fxtal) / t;
        c = Fxtal / t;
    }

    pll_conf->mult = a;
    pll_conf->num = b;
    pll_conf->denom = c;
    out_conf->div = x;
    out_conf->num = y;
    out_conf->denom = z;
}

/*
 * Initializes Si5351. Call this function before doing anything else.
 * `Correction` is the difference of actual frequency an desired frequency @ 100 MHz.
 * It can be measured at lower frequencies and scaled linearly.
 * E.g. if you get 10_000_097 Hz instead of 10_000_000 Hz, `correction` is 97*10 = 970
 */
void si5351_init(uint32_t i2c, int32_t correction) {
    si5351Correction = correction;

    // Disable all outputs by setting CLKx_DIS high
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL, 0xFF);

    // Power down all output drivers
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_16_CLK0_CONTROL, 0x80);
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_17_CLK1_CONTROL, 0x80);
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_18_CLK2_CONTROL, 0x80);
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_19_CLK3_CONTROL, 0x80);
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_20_CLK4_CONTROL, 0x80);
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_21_CLK5_CONTROL, 0x80);
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_22_CLK6_CONTROL, 0x80);
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_23_CLK7_CONTROL, 0x80);

    // Set the load capacitance for the XTAL
    si5351_crystal_load_t crystalLoad = SI5351_CRYSTAL_LOAD_10PF;
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_183_CRYSTAL_INTERNAL_LOAD_CAPACITANCE, crystalLoad);
}

void si5351_setup_pll(uint32_t i2c, si5351_pll_t pll, si5351_pll_cfg* conf) {
    int32_t P1, P2, P3;
    int32_t mult = conf->mult;
    int32_t num = conf->num;
    int32_t denom = conf->denom;

    P1 = 128 * mult + (128 * num)/denom - 512;
    // P2 = 128 * num - denom * ((128 * num)/denom);
    P2 = (128 * num) % denom;
    P3 = denom;

    // Get the appropriate base address for the PLL registers
    uint8_t baseaddr = (pll == SI5351_PLL_A ? 26 : 34);
    si5351_write_bulk(i2c, baseaddr, P1, P2, P3, 0, 0);

    // Reset both PLLs
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_177_PLL_RESET, (1<<7) | (1<<5) );
}

int si5351_setup_output(
		uint32_t i2c,
		uint8_t output, 
		si5351_pll_t pllSource, 
		si5351_drv_strength_t driveStrength, 
		si5351_out_conf* conf, 
		uint8_t phaseOffset) {
    int32_t div = conf->div;
    int32_t num = conf->num;
    int32_t denom = conf->denom;
    uint8_t divBy4 = 0;
    int32_t P1, P2, P3;

    if(output > 2) {
        return 1;
    }

    if((!conf->allowIntegerMode) && ((div < 8) || ((div == 8) && (num == 0)))) {
        // div in { 4, 6, 8 } is possible only in integer mode
        return 2;
    }

    if(div == 4) {
        // special DIVBY4 case, see AN619 4.1.3
        P1 = 0;
        P2 = 0;
        P3 = 1;
        divBy4 = 0x3;
    } else {
        P1 = 128 * div + ((128 * num)/denom) - 512;
        // P2 = 128 * num - denom * (128 * num)/denom;
        P2 = (128 * num) % denom;
        P3 = denom;
    }

    // Get the register addresses for given channel
    uint8_t baseaddr = 0;
    uint8_t phaseOffsetRegister = 0;
    uint8_t clkControlRegister = 0;
    switch (output) {
    case 0:
        baseaddr = SI5351_REGISTER_42_MULTISYNTH0_PARAMETERS_1;
        phaseOffsetRegister = SI5351_REGISTER_165_CLK0_INITIAL_PHASE_OFFSET;
        clkControlRegister = SI5351_REGISTER_16_CLK0_CONTROL;
        break;
    case 1:
        baseaddr = SI5351_REGISTER_50_MULTISYNTH1_PARAMETERS_1;
        phaseOffsetRegister = SI5351_REGISTER_166_CLK1_INITIAL_PHASE_OFFSET;
        clkControlRegister = SI5351_REGISTER_17_CLK1_CONTROL;
        break;
    case 2:
        baseaddr = SI5351_REGISTER_58_MULTISYNTH2_PARAMETERS_1;
        phaseOffsetRegister = SI5351_REGISTER_167_CLK2_INITIAL_PHASE_OFFSET;
        clkControlRegister = SI5351_REGISTER_18_CLK2_CONTROL;
        break;
    }

    uint8_t clkControl = 0x0C | driveStrength; // clock not inverted, powered up
    if(pllSource == SI5351_PLL_B) {
        clkControl |= (1 << 5); // Uses PLLB
    }

    if((conf->allowIntegerMode) && ((num == 0)||(div == 4))) {
        // use integer mode
        clkControl |= (1 << 6);
    }

    i2c_write_reg_single(i2c, SI5351_ADDRESS, clkControlRegister, clkControl);
    si5351_write_bulk(i2c, baseaddr, P1, P2, P3, divBy4, conf->rdiv);
    i2c_write_reg_single(i2c, SI5351_ADDRESS, phaseOffsetRegister, (phaseOffset & 0x7F));

    return 0;
}

void si5351_setup_clk0(uint32_t i2c, int32_t Fclk, si5351_drv_strength_t driveStrength) {
	si5351_pll_cfg pll_conf;
	si5351_out_conf out_conf;

	si5351_calculate_params(Fclk, &pll_conf, &out_conf);
	si5351_setup_pll(i2c, SI5351_PLL_A, &pll_conf);
	si5351_setup_output(i2c, 0, SI5351_PLL_A, driveStrength, &out_conf, 0);
}
void si5351_setup_clk1(uint32_t i2c, int32_t Fclk, si5351_drv_strength_t driveStrength) {
	si5351_pll_cfg pll_conf;
	si5351_out_conf out_conf;

	si5351_calculate_params(Fclk, &pll_conf, &out_conf);
	si5351_setup_pll(i2c, SI5351_PLL_A, &pll_conf);
	si5351_setup_output(i2c, 1, SI5351_PLL_A, driveStrength, &out_conf, 0);
}
void si5351_setup_clk2(uint32_t i2c, int32_t Fclk, si5351_drv_strength_t driveStrength) {
	si5351_pll_cfg pll_conf;
	si5351_out_conf out_conf;

	si5351_calculate_params(Fclk, &pll_conf, &out_conf);
	si5351_setup_pll(i2c, SI5351_PLL_B, &pll_conf);
	si5351_setup_output(i2c, 2, SI5351_PLL_B, driveStrength, &out_conf, 0);
}

void si5351_enable_outputs(uint32_t i2c, uint8_t enabled) {
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL, ~enabled);
}

void si5351_disable_all_outputs(uint32_t i2c) {
    i2c_write_reg_single(i2c, SI5351_ADDRESS, SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL, 0xFF);
}

void si5351_write_bulk(uint32_t i2c, uint8_t baseaddr, int32_t P1, int32_t P2, int32_t P3, uint8_t divBy4, si5351_rdiv_t rdiv) {
    uint8_t bulk[8] = {0};
    bulk[0] = (P3 >> 8) & 0xFF;
    bulk[1] = P3 & 0xFF;
    bulk[2] = ((P1 >> 16) & 0x3) | ((divBy4 & 0x3) << 2) | ((rdiv & 0x7) << 4);
    bulk[3] = (P1 >> 8) & 0xFF;
    bulk[4] = P1 & 0xFF;
    bulk[5] = ((P3 >> 12) & 0xF0) | ((P2 >> 16) & 0xF);
    bulk[6] = (P2 >> 8) & 0xFF;
    bulk[7] = P2 & 0xFF;
    i2c_write_reg(i2c, SI5351_ADDRESS, baseaddr, bulk, 8);
}
