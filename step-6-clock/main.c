#include "derivative.h"
#include "systick.h"
#include "uart.h"
#include <stdio.h>

int main(void) {
    // Initialize
    systick_init(CORCLK / 1000);  // Period of systick timer : 1ms
    uart_init(UART_MSG, 9600);    // Buad rate : 9600

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

    printf("Hello world!\r\n");
    printf("Clock: %lu\r\n", CORCLK);

    uint32_t timer = 0;
    for (;;) {
        if (timer_expired(&timer, 1000)) {
            GPIOC->PDOR = ~GPIOC->PDOR;
            printf("LED: %d, tick: %lu\r\n", GPIOC->PDOR & BIT(12) ? 1 : 0, ms_ticks);
        }
    }
}
