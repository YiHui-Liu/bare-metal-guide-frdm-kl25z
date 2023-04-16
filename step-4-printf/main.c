#include "MKL25Z4.h"
#include "systick.h"
#include "uart.h"
#include <stdio.h>

#define PI 3.1415926
#define BIT(x) (1UL << (x))

void SysTick_Handler(void) { ms_ticks++; }

int main(void) {
    // Initialize
    systick_init(CORCLK / 1000);      // Period of systick timer : 1ms
    uart_init(UART1_BASE_PTR, 9600);  // Buad rate : 9600

    // Enable clock for PORTC and PORTB
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTB_MASK;

    // Set PORTC12, PORTC9, PORTB18, PORTB19 as GPIO
    PORTC_PCR12 = PORT_PCR_MUX(0x1);

    PORTC_PCR9 = PORT_PCR_MUX(0x1);
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
    GPIOC_PDOR |= BIT(12);

    char buffer[64];
    printf("Hello world!\r\n");
    printf("Please enter a name:\n\r");
    scanf("%s", buffer);
    printf("  I have received: '%s'\r\n", buffer);

    uint32_t timer = 0;
    for (;;) {
        if (timer_expired(&timer, 1000)) {
            GPIOC_PDOR = ~GPIOC_PDOR;
            printf("LED: %d, tick: %lu\r\n", GPIOC_PDOR & BIT(12) ? 1 : 0, ms_ticks);
        }
    }
}
