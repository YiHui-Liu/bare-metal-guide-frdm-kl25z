#include "derivative.h"
#include "uart.h"

uint32_t CORCLK = CORCLK_DEFAULT;
uint32_t BUSCLK = BUSCLK_DEFAULT;

// ms count, volatile is important!!
volatile uint32_t ms_ticks;

void SysTick_Handler(void) { ms_ticks++; }

void UART1_IRQHandler(void) {
    if (UART1->S1 & UART_S1_RDRF_MASK) {
        char buf[64];
        size_t len = uart_getline(UART1, buf);
        uart_printf(UART_MSG, "%d: ", len);
        uart_write_buf(UART_MSG, buf, len);
        buf[0] = UART1->D;
    }
}
