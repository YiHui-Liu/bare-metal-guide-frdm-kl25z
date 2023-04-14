#include "MKL25Z4.h"
#include <inttypes.h>
#include <stdbool.h>

#define BIT(x) (1UL << (x))
#define KINETIS_WDOG_DISABLED_CTRL 0x0

static inline void spin(volatile uint32_t count) {
    while (count--) asm("nop");
}

int main(void) {
    // Enable clock for PORTC and PORTB
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTB_MASK;

    // Set PORTC12, PORTC9, PORTB18, PORTB19 as GPIO
    PORTC_PCR9 = PORT_PCR_MUX(0x1);
    PORTC_PCR12 = PORT_PCR_MUX(0x1);
    PORTB_PCR18 = PORT_PCR_MUX(0x1);
    PORTB_PCR19 = PORT_PCR_MUX(0x1);

    // R
    GPIOC_PDDR |= BIT(9);
    GPIOC_PDOR |= BIT(9);

    // B
    GPIOB_PDDR |= BIT(18);
    GPIOB_PDOR |= BIT(18);

    // G
    GPIOB_PDDR |= BIT(19);
    GPIOB_PDOR |= BIT(19);

    // turn on or off LED
    GPIOC_PDDR |= BIT(12);
    GPIOC_PDOR &= ~BIT(12);

    for (;;) {
        spin(999999);
        GPIOC_PDOR = ~GPIOC_PDOR;
    }
}

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

extern void _estack(void);

/*
    Set tab (the vector table) in the section ".vectors"
    and the size of the vector table is 16 + 33
    the first 16 * 4 bytes are reserved for the Cortex-M0+ core
    the first one is the initial stack pointer
    the second one is the initial program counter
*/
__attribute__((section(".vectors"))) void (*tab[16 + 33])(void) = {_estack, _reset};
