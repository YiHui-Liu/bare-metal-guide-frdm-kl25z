#include "derivative.h"

uint32_t CORCLK = CORCLK_DEFAULT;
uint32_t BUSCLK = BUSCLK_DEFAULT;

// ms count, volatile is important!!
volatile uint32_t ms_ticks;

void SysTick_Handler(void) { ms_ticks++; }
