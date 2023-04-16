#include "MKL25Z4.h"
#include <string.h>

#define KINETIS_WDOG_DISABLED_CTRL 0x0

extern int main(void);  // Defined in main.c

static void __init_hardware() {
    // Switch off watchdog
    SIM_COPC = KINETIS_WDOG_DISABLED_CTRL;
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

// Interrupt service routine (ISR)
extern void _estack(void);          // Defined in link.ld
extern void SysTick_Handler(void);  // Defined in main.c

void Default_Handler() { __asm("bkpt"); }

/*
    Set tab (the vector table) in the section ".vectors"
    and the size of the vector table is 16 + 32
    the first 16 * 4 bytes are reserved for the Cortex-M0+ core
    the first one is the initial stack pointer
    the second one is the initial program counter
*/

__attribute__((section(".vectors"))) void (*tab[16 + 32])(void) = {
    _estack,          // Initial stack pointer
    _reset,           // Reset handler
    Default_Handler,  // NMI handler
    Default_Handler,  // Hard Fault handler
    0,                // Reserved
    0,                // Reserved
    0,                // Reserved
    0,                // Reserved
    0,                // Reserved
    0,                // Reserved
    0,                // Reserved
    Default_Handler,  // SVC handler
    0,                // Reserved
    0,                // Reserved
    Default_Handler,  // PendSV handler
    SysTick_Handler   // SysTick handler
};

__attribute__((section(".cfmconfig"))) uint32_t(cfm[4]) = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE};
