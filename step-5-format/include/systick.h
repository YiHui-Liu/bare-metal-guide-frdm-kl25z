#pragma once

#include "derivative.h"
#include <stdbool.h>

extern volatile uint32_t ms_ticks;

static inline void spin(volatile uint32_t count) {
    while (count--) asm("nop");
}

// t: expiration time, prd: period
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
