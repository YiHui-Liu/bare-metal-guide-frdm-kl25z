#include "derivative.h"
#include "systick.h"

uint32_t CORCLK = CORCLK_DEFAULT;
uint32_t BUSCLK = BUSCLK_DEFAULT;

void SysTick_Handler(void) { ms_ticks++; }
