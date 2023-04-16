#include "MKL25Z4.h"
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#define BIT(x) (1UL << (x))
#define KINETIS_WDOG_DISABLED_CTRL 0x0

static inline void spin(volatile uint32_t count) {
    while (count--) asm("nop");
}

static inline void systick_init(uint32_t ticks) {
    if ((ticks - 1) > 0xffffff) return;  // Systick timer is 24 bit
    SYST_RVR = ticks - 1;
    SYST_CVR = 0;
    SYST_CSR = SysTick_CSR_ENABLE_MASK        // Enable systick timer
               | SysTick_CSR_TICKINT_MASK     // Enable interrupt
               | SysTick_CSR_CLKSOURCE_MASK;  // Use butin-in clock
}

static volatile uint32_t ms_ticks;  // volatile is important!!
void SysTick_Handler(void) { ms_ticks++; }

// t: expiration time, prd: period Return true if expired
bool timer_expired(uint32_t *t, uint32_t prd) {
    if (ms_ticks + prd < *t) *t = 0;                         // Time wrapped? Reset timer
    if (*t == 0) *t = ms_ticks + prd;                        // First poll? Set expiration
    if (*t > ms_ticks) return false;                         // Not expired yet, return
    *t = (ms_ticks - *t) > prd ? ms_ticks + prd : *t + prd;  // Next expiration time
    return true;                                             // Expired, return true
}

int main(void) {
    uint32_t timer = 0;
    systick_init(48000000 / 1000);  // 1ms

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
        if (timer_expired(&timer, 400)) {
            GPIOC_PDOR = ~GPIOC_PDOR;
        }
    }
}

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

extern void _estack(void);

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
