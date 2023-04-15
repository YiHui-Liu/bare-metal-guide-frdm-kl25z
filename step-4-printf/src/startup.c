#include "MKL25Z4.h"

#define KINETIS_WDOG_DISABLED_CTRL 0x0

extern int main(void);  // Defined in main.c

void __init_hardware() {
    // Switch off watchdog
    SIM_COPC = KINETIS_WDOG_DISABLED_CTRL;
}

__attribute__((naked, noreturn)) void _reset(void) {
    __init_hardware();

    extern long _sbss, _ebss, _sdata, _edata, _sidata;
    for (long *src = &_sbss; src < &_ebss; src++) *src = 0;
    for (long *src = &_sdata, *dst = &_sidata; src < &_edata; src++, dst++) *src = *dst;

    main();
    for (;;) (void)0;
}

// Interrupt service routine (ISR)
extern void _estack(void);          // Defined in link.ld
extern void SysTick_Handler(void);  // Defined in main.c

/*
    Set tab (the vector table) in the section ".vectors"
    and the size of the vector table is 16 + 32
    the first 16 * 4 bytes are reserved for the Cortex-M0+ core
    the first one is the initial stack pointer
    the second one is the initial program counter
*/
__attribute__((section(".vectors"))) void (*tab[16 + 32])(void) = {_estack, _reset, 0, 0, 0, 0, 0, 0,
                                                                   0,       0,      0, 0, 0, 0, 0, SysTick_Handler};

__attribute__((section(".cfmconfig"))) uint32_t(cfm[4]) = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE};
