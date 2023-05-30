#include "derivative.h"
#include "systick.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    // Initialize
    SysTick_Config(CORCLK / 1000);  // Period of systick timer : 1ms
    uart_init(UART_MSG, 9600);      // Initialize UART1 with PC
    uart_rie_enable(UART_MSG);      // Enable UART1 receive interrupt

    uart_printf(UART_MSG, "System Clock: %lu\r\n", CORCLK);
    uart_printf(UART_MSG, "Bus Clock: %lu\r\n", BUSCLK);

    uint32_t timer = 0;
    for (;;) {
        if (timer_expired(&timer, 1000)) {
            uart_printf(UART_MSG, "UART RD: %d, tick: %lu\r\n", UART_MSG->S1 & UART_S1_RDRF_MASK ? 1 : 0, ms_ticks);
        }
    }
}
