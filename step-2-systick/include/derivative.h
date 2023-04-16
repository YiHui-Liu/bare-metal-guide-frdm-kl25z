#pragma once

#include "MKL25Z4.h"

#define CORCLK_DEFAULT 20970000  // Default Core clock, 20.97 Mhz

#define BIT(x) (1UL << (x))

extern uint32_t CORCLK;
