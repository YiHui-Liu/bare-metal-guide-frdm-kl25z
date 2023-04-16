#pragma once

#include "derivative.h"
#include <stdbool.h>

static inline void spin(volatile uint32_t count) {
    if (count == 0) return;
    while (count--) asm("nop");
}

// ms count, volatile is important!!
static volatile uint32_t ms_ticks;

static inline void systick_init(uint32_t ticks) {
    if ((ticks - 1) > 0xffffff) return;  // Systick timer is 24 bit
    SysTick->LOAD = ticks - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk        // Enable systick timer
                    | SysTick_CTRL_TICKINT_Msk     // Enable interrupt
                    | SysTick_CTRL_CLKSOURCE_Msk;  // Use butin-in clock
}

// t: expiration time, prd: period Return true if expired
static inline bool timer_expired(uint32_t *t, uint32_t prd) {
    if (ms_ticks + prd < *t) *t = 0;                         // Time wrapped? Reset timer
    if (*t == 0) *t = ms_ticks + prd;                        // First poll? Set expiration
    if (*t > ms_ticks) return false;                         // Not expired yet, return
    *t = (ms_ticks - *t) > prd ? ms_ticks + prd : *t + prd;  // Next expiration time
    return true;                                             // Expired, return true
}

static inline void delay_ms(uint32_t ms) {
    uint32_t start = ms_ticks;
    while (ms_ticks - start < ms) asm("nop");
}
