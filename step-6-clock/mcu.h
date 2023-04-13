// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#pragma once

#include "stm32f429xx.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define BIT(x) (1UL << (x))
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

// 6.3.3: APB1 clock <= 45MHz; APB2 clock <= 90MHz
// 3.5.1, Table 11: configure flash latency (WS) in accordance to clock freq
// 33.4: The AHB clock must be at least 25 MHz when Ethernet is used
enum { APB1_PRE = 5 /* AHB clock / 4 */, APB2_PRE = 4 /* AHB clock / 2 */ };
enum { PLL_HSI = 16, PLL_M = 8, PLL_N = 180, PLL_P = 2 };  // Run at 180 Mhz
#define FLASH_LATENCY 5
#define SYS_FREQUENCY ((PLL_HSI * PLL_N / PLL_M / PLL_P) * 1000000)
#define APB2_FREQUENCY (SYS_FREQUENCY / (BIT(APB2_PRE - 3)))
#define APB1_FREQUENCY (SYS_FREQUENCY / (BIT(APB1_PRE - 3)))

static inline void spin(volatile uint32_t count) {
    while (count--) asm("nop");
}

static inline void systick_init(uint32_t ticks) {
    if ((ticks - 1) > 0xffffff) return;  // Systick timer is 24 bit
    SysTick->LOAD = ticks - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = BIT(0) | BIT(1) | BIT(2);  // Enable systick
    RCC->APB2ENR |= BIT(14);                   // Enable SYSCFG
}

#define GPIO(bank) ((GPIO_TypeDef *)(GPIOA_BASE + 0x400U * (bank)))
enum { GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG };

static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
    GPIO_TypeDef *gpio = GPIO(PINBANK(pin));  // GPIO bank
    int n = PINNO(pin);                       // Pin number
    RCC->AHB1ENR |= BIT(PINBANK(pin));        // Enable GPIO clock
    gpio->MODER &= ~(3U << (n * 2));          // Clear existing setting
    gpio->MODER |= (mode & 3U) << (n * 2);    // Set new mode
}

static inline void gpio_set_af(uint16_t pin, uint8_t af_num) {
    GPIO_TypeDef *gpio = GPIO(PINBANK(pin));  // GPIO bank
    int n = PINNO(pin);                       // Pin number
    gpio->AFR[n >> 3] &= ~(15UL << ((n & 7) * 4));
    gpio->AFR[n >> 3] |= ((uint32_t)af_num) << ((n & 7) * 4);
}

static inline void gpio_write(uint16_t pin, bool val) {
    GPIO_TypeDef *gpio = GPIO(PINBANK(pin));
    gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
}

#define UART1 USART1
#define UART2 USART2
#define UART3 USART3

static inline void uart_init(USART_TypeDef *uart, unsigned long baud) {
    // https://www.st.com/resource/en/datasheet/stm32f429zi.pdf
    uint8_t af = 7;           // Alternate function
    uint16_t rx = 0, tx = 0;  // pins
    uint32_t freq = 0;        // Bus frequency. UART1 is on APB2, rest on APB1

    if (uart == UART1) freq = APB2_FREQUENCY, RCC->APB2ENR |= BIT(4);
    if (uart == UART2) freq = APB1_FREQUENCY, RCC->APB1ENR |= BIT(17);
    if (uart == UART3) freq = APB1_FREQUENCY, RCC->APB1ENR |= BIT(18);

    if (uart == UART1) tx = PIN('A', 9), rx = PIN('A', 10);
    if (uart == UART2) tx = PIN('A', 2), rx = PIN('A', 3);
    if (uart == UART3) tx = PIN('D', 8), rx = PIN('D', 9);

    gpio_set_mode(tx, GPIO_MODE_AF);
    gpio_set_af(tx, af);
    gpio_set_mode(rx, GPIO_MODE_AF);
    gpio_set_af(rx, af);
    uart->CR1 = 0;                           // Disable this UART
    uart->BRR = freq / baud;                 // Set baud rate
    uart->CR1 |= BIT(13) | BIT(2) | BIT(3);  // Set UE, RE, TE
}

static inline void uart_write_byte(USART_TypeDef *uart, uint8_t byte) {
    uart->DR = byte;
    while ((uart->SR & BIT(7)) == 0) spin(1);
}

static inline void uart_write_buf(USART_TypeDef *uart, char *buf, size_t len) {
    while (len-- > 0) uart_write_byte(uart, *(uint8_t *)buf++);
}

static inline int uart_read_ready(USART_TypeDef *uart) {
    return uart->SR & BIT(5);  // If RXNE bit is set, data is ready
}

static inline uint8_t uart_read_byte(USART_TypeDef *uart) { return (uint8_t)(uart->DR & 255); }

static inline bool timer_expired(uint32_t *t, uint32_t prd, uint32_t now) {
    if (now + prd < *t) *t = 0;                    // Time wrapped? Reset timer
    if (*t == 0) *t = now + prd;                   // Firt poll? Set expiration
    if (*t > now) return false;                    // Not expired yet, return
    *t = (now - *t) > prd ? now + prd : *t + prd;  // Next expiration time
    return true;                                   // Expired, return true
}

static inline void clock_init(void) {                   // Set clock frequency
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));  // Enable FPU
    FLASH->ACR |= FLASH_LATENCY | BIT(8) | BIT(9);      // Flash latency, caches
    RCC->PLLCFGR &= ~((BIT(17) - 1));                   // Clear PLL multipliers
    RCC->PLLCFGR |= (((PLL_P - 2) / 2) & 3) << 16;      // Set PLL_P
    RCC->PLLCFGR |= PLL_M | (PLL_N << 6);               // Set PLL_M and PLL_N
    RCC->CR |= BIT(24);                                 // Enable PLL
    while ((RCC->CR & BIT(25)) == 0) spin(1);           // Wait until done
    RCC->CFGR = (APB1_PRE << 10) | (APB2_PRE << 13);    // Set prescalers
    RCC->CFGR |= 2;                                     // Set clock source to PLL
    while ((RCC->CFGR & 12) == 0) spin(1);              // Wait until done
}
