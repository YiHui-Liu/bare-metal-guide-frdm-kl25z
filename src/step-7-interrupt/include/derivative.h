#pragma once

#include "MKL25Z4.h"

// Define Clock Settings
#define CORCLK_DEFAULT 20970000u  // Default Core clock, 20.97 Mhz
#define BUSCLK_DEFAULT 10500000u  // Default Bus Rate clock, 10.5 Mhz, half of CORCLK

#define CPU_XTAL_CLK_HZ 8000000u      // Value of the external crystal or oscillator clock frequency in Hz
#define CPU_INT_SLOW_CLK_HZ 32768u    // Value of the slow internal oscillator clock frequency in Hz
#define CPU_INT_FAST_CLK_HZ 4000000u  // Value of the fast internal oscillator clock frequency in Hz

#define UART_MSG UART1

#define PI 3.1415926
#define BIT(x) (1UL << (x))

extern uint32_t CORCLK;
extern uint32_t BUSCLK;
extern volatile uint32_t ms_ticks;
extern void SysTick_Handler(void);
extern void UART1_IRQHandler(void);
