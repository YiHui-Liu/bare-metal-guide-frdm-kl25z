#pragma once

#include "MKL25Z4.h"

#define CORCLK_DEFAULT 20970000  // Default Core clock, 20.97 Mhz
#define BUSCLK_DEFAULT 10500000  // Default Bus Rate clock, 10.5 Mhz, half of CORCLK

#define UART_MSG UART1

#define PI 3.1415926
#define BIT(x) (1UL << (x))

extern uint32_t CORCLK;
extern uint32_t BUSCLK;
