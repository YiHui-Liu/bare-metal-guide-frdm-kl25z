#include "derivative.h"
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

static inline void spin(volatile uint32_t count) {
    while (count--) asm("nop");
}

int main(void) {
    // Enable clock for PORTC and PORTB
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTB_MASK;

    // Set PORTC12, PORTC9, PORTB18, PORTB19 as GPIO
    PORTC->PCR[12] = PORT_PCR_MUX(0x1);

    PORTC->PCR[9] = PORT_PCR_MUX(0x1);
    PORTB->PCR[18] = PORT_PCR_MUX(0x1);
    PORTB->PCR[19] = PORT_PCR_MUX(0x1);

    // R
    GPIOC->PDDR |= BIT(9);
    GPIOC->PDOR |= BIT(9);

    // B
    GPIOB->PDDR |= BIT(18);
    GPIOB->PDOR |= BIT(18);

    // G
    GPIOB->PDDR |= BIT(19);
    GPIOB->PDOR |= BIT(19);

    // turn on or off LED
    GPIOC->PDDR |= BIT(12);
    GPIOC->PDOR |= BIT(12);

    for (;;) {
        spin(999999);
        GPIOC->PDOR = ~GPIOC->PDOR;
    }
}

static void __init_hardware() {
    // Switch off watchdog
    SIM->COPC = 0x0;
}

static void zero_fill_bss(void) {
    extern char _sbss[];
    extern char _ebss[];
    memset(_sbss, 0, (size_t)(_ebss - _sbss));
}

static void copy_data(void) {
    extern char _sdata[];
    extern char _edata[];
    extern char _sidata[];
    memcpy(_sdata, _sidata, (size_t)(_edata - _sdata));
}

__attribute__((naked, noreturn)) void _reset(void) {
    __init_hardware();
    zero_fill_bss();
    copy_data();

    main();
    for (;;) (void)0;
}

extern void _estack(void);

/*
    Set tab (the vector table) in the section ".vectors"
    and the size of the vector table is 16 + 32
    the first 16 * 4 bytes are reserved for the Cortex-M0+ core
    the first one is the initial stack pointer
    the second one is the initial program counter
*/
__attribute__((section(".vectors"))) void (*tab[16 + 32])(void) = {_estack, _reset};

__attribute__((section(".cfmconfig"))) uint32_t(cfm[4]) = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE};
