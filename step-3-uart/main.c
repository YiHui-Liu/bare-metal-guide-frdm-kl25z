#include "MKL25Z4.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#define BIT(x) (1UL << (x))
#define BUSCLK 10500000  // Bus Rate clock, 10.5 Mhz
// #define BUSCLK 24000000  // Bus Rate clock, 24 Mhz
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

static inline void uart1_init(unsigned long baud) {
    // Enable clock for UART and PORT
    SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;

    // Set PORTC3, PORTC4 as RXD, TXD
    PORTC_PCR3 = PORT_PCR_MUX(0x3);
    PORTC_PCR4 = PORT_PCR_MUX(0x3);

    // Make sure that the transmitter and receiver are disabled while we change settings.
    UART1_C2 &= (uint8_t)(~(UART_C2_TE_MASK | UART_C2_RE_MASK));

    // default settings, no parity, so entire register is cleared
    UART1_C1 = 0x00;

    // UART use the bus-rate clock(BUSCLK)
    // Buad = BUSCLK / (16 * SBR)
    unsigned short sbr = (unsigned short)(BUSCLK / (baud * 16));

    // UARTx_BDH bits 0~4 is the high 5 bits of SBR (band rate)
    UART1_BDH |= (sbr & UART_BDH_SBR_MASK << 8) >> 8;
    // UARTx_BLH is the low 8 bits of SBR (band rate)
    UART1_BDL = sbr & UART_BDL_SBR_MASK;
    // Enable receiver and transmitter
    UART1_C2 |= UART_C2_TE_MASK      // Transmitter enable
                | UART_C2_RE_MASK    // Receiver enable
                | UART_C2_RIE_MASK;  // Receiver interrupt enable
}

static inline int uart1_read_ready() {
    // Receive Data Register Full Flag (RDRF): set when the receive data buffer is full
    return UART1_S1 & UART_S1_RDRF_MASK;
}

static inline uint8_t uart1_read_byte() { return (uint8_t)UART1_D; }

static inline void uart1_write_byte(uint8_t byte) {
    // Transmit Data Register Empty Flag (TDRE): set when the transmit data buffer is empty
    UART1_D = byte;
    while (!(UART1_S1 & UART_S1_TDRE_MASK)) spin(1);
}

static inline void uart1_write_buf(char *buf, size_t len) {
    while (len-- > 0) uart1_write_byte(*(uint8_t *)buf++);
}

int main(void) {
    uint32_t timer = 0;
    systick_init(48000000 / 1000);  // 1ms
    uart1_init(9600);

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
        if (timer_expired(&timer, 100)) {
            GPIOC_PDOR = ~GPIOC_PDOR;
            uart1_write_buf("hi\r\n", 4);
        }
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
    and the size of the vector table is 16 + 32
    the first 16 * 4 bytes are reserved for the Cortex-M0+ core
    the first one is the initial stack pointer
    the second one is the initial program counter
*/
__attribute__((section(".vectors"))) void (*tab[16 + 32])(void) = {_estack, _reset, 0, 0, 0, 0, 0, 0,
                                                                   0,       0,      0, 0, 0, 0, 0, SysTick_Handler};
