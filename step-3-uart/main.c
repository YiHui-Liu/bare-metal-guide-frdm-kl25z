#include "derivative.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern volatile uint32_t ms_ticks;
extern void SysTick_Handler(void);

static inline void spin(volatile uint32_t count) {
    while (count--) asm("nop");
}

static inline void systick_init(uint32_t ticks) {
    if ((ticks - 1) > 0xffffff) return;  // Systick timer is 24 bit
    SysTick->LOAD = ticks - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk        // Enable systick timer
                    | SysTick_CTRL_TICKINT_Msk     // Enable interrupt
                    | SysTick_CTRL_CLKSOURCE_Msk;  // Use butin-in clock
}

// t: expiration time, prd: period Return true if expired
bool timer_expired(uint32_t *t, uint32_t prd) {
    if (ms_ticks + prd < *t) *t = 0;                         // Time wrapped? Reset timer
    if (*t == 0) *t = ms_ticks + prd;                        // First poll? Set expiration
    if (*t > ms_ticks) return false;                         // Not expired yet, return
    *t = (ms_ticks - *t) > prd ? ms_ticks + prd : *t + prd;  // Next expiration time
    return true;                                             // Expired, return true
}

static inline void uart_init(UART_Type *UART, unsigned long baud) {
    // Enable clock for UART and PORT, then set RXD, TXD
    if (UART == UART1) {
        SIM->SCGC4 |= SIM_SCGC4_UART1_MASK;
        SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;

        PORTC->PCR[3] = PORT_PCR_MUX(0x3);
        PORTC->PCR[4] = PORT_PCR_MUX(0x3);
    } else if (UART == UART2) {
        SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
        SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;

        PORTD->PCR[2] = PORT_PCR_MUX(0x3);
        PORTD->PCR[3] = PORT_PCR_MUX(0x3);
    } else
        return;

    // Make sure that the transmitter and receiver are disabled while we change settings.
    UART->C2 &= (uint8_t)(~(UART_C2_TE_MASK | UART_C2_RE_MASK));

    // default settings, no parity, so entire register is cleared
    UART->C1 = 0x00;

    // UART use the bus-rate clock(BUSCLK)
    // Buad = BUSCLK / (16 * SBR)
    unsigned short sbr = (unsigned short)(BUSCLK / (baud * 16));

    // UARTx_BDH bits 0~4 is the high 5 bits of SBR (band rate)
    UART->BDH |= (sbr & (uint8_t)(UART_BDH_SBR_MASK << 8)) >> 8;
    // UARTx_BLH is the low 8 bits of SBR (band rate)
    UART->BDL = sbr & UART_BDL_SBR_MASK;
    // Enable receiver and transmitter
    UART->C2 |= UART_C2_TE_MASK      // Transmitter enable
                | UART_C2_RE_MASK    // Receiver enable
                | UART_C2_RIE_MASK;  // Receiver interrupt enable
}

static inline int uart_read_ready(UART_Type *UART) {
    // Receive Data Register Full Flag (RDRF): set when the receive data buffer is full
    return UART->S1 & UART_S1_RDRF_MASK;
}

static inline uint8_t uart_read_byte(UART_Type *UART) { return (uint8_t)UART->D; }

static inline void uart_write_byte(UART_Type *UART, uint8_t byte) {
    // Transmit Data Register Empty Flag (TDRE): set when the transmit data buffer is empty
    while (!(UART->S1 & UART_S1_TDRE_MASK)) asm("nop");
    UART->D = byte;
}

static inline void uart_write_buf(UART_Type *UART, char *buf, size_t len) {
    while (len-- > 0) uart_write_byte(UART, *(uint8_t *)buf++);
}

int main(void) {
    uint32_t timer = 0;
    systick_init(48000000 / 1000);  // 1ms
    uart_init(UART_MSG, 9600);      // Buad rate : 9600

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
        if (timer_expired(&timer, 100)) {
            GPIOC->PDOR = ~GPIOC->PDOR;
            uart_write_buf(UART_MSG, "hi\r\n", 4);
        }
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
__attribute__((section(".vectors"))) void (*tab[16 + 32])(void) = {_estack, _reset, 0, 0, 0, 0, 0, 0,
                                                                   0,       0,      0, 0, 0, 0, 0, SysTick_Handler};

__attribute__((section(".cfmconfig"))) uint32_t(cfm[4]) = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE};
