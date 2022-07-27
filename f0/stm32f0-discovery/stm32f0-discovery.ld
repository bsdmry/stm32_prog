/* Linker script for ST STM32F0DISCOVERY (STM32F051R8T6, 64K flash, 8K RAM). */

/* Define memory regions. */
MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 64K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 8K
}
/* Include the common ld script. */
INCLUDE cortex-m-generic.ld
