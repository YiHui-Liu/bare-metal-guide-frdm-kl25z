#pragma once

#include "derivative.h"
#include "systick.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static inline void uart_init(UART_Type *UART, unsigned long baud) {
    // Enable clock for UART and PORT, then set RXD, TXD
    if (UART == UART1) {
        SIM->SCGC4 |= SIM_SCGC4_UART1_MASK;
        SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;

        PORTC->PCR[3] = PORT_PCR_MUX(0x3);
        PORTC->PCR[4] = PORT_PCR_MUX(0x3);
    } else if (UART == UART2) {
        SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
        SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

        PORTE->PCR[23] = PORT_PCR_MUX(0x3);
        PORTE->PCR[22] = PORT_PCR_MUX(0x3);
    } else
        return;

    // Make sure that the transmitter and receiver are disabled while we change settings.
    UART->C2 &= (uint8_t)(~(UART_C2_TE_MASK | UART_C2_RE_MASK));

    // default settings, no parity, so entire register is cleared
    UART->C1 = 0x00;

    // Buad = BUSCLK / (16 * SBR)
    unsigned short sbr = (unsigned short)(BUSCLK / (baud * 16));

    // UARTx_BDH bits 0~4 is the high 5 bits of SBR (band rate)
    UART->BDH |= (sbr & (uint8_t)(UART_BDH_SBR_MASK << 8)) >> 8;
    // UARTx_BLH is the low 8 bits of SBR (band rate)
    UART->BDL = sbr & UART_BDL_SBR_MASK;
    // Enable receiver and transmitter
    UART->C2 |= UART_C2_TE_MASK     // Transmitter enable
                | UART_C2_RE_MASK;  // Receiver enable
}

static inline void uart_rie_enable(UART_Type *UART) {
    // Enable UART interrupt
    if (UART == UART1)
        NVIC_EnableIRQ(UART1_IRQn);
    else if (UART == UART2)
        NVIC_EnableIRQ(UART2_IRQn);
    UART->C2 |= UART_C2_RIE_MASK;  // Receiver interrupt enable
}

static inline int uart_read_ready(UART_Type *UART) {
    // Receive Data Register Full Flag (RDRF): set when the receive data buffer is full
    return UART->S1 & UART_S1_RDRF_MASK;
}

static inline uint8_t uart_read_byte(UART_Type *UART) { return (uint8_t)UART->D; }

static inline void uart_write_byte(UART_Type *UART, uint8_t byte) {
    // Transmit Data Register Empty Flag (TDRE): set when the transmit data buffer is empty
    while (!(UART->S1 & UART_S1_TDRE_MASK)) asm("nop");
    UART->D = byte;
}

static inline void uart_write_buf(UART_Type *UART, char *buf, size_t len) {
    while (len-- > 0) uart_write_byte(UART, *(uint8_t *)buf++);
}

static inline void uart_printf(UART_Type *UART, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buf[64];
    vsprintf(buf, format, args);
    uart_write_buf(UART, buf, strlen(buf));
    va_end(args);
}

static inline size_t uart_getline(UART_Type *UART, char *buf) {
    size_t cnt = 0;
    while (1) {
        while (!uart_read_ready(UART)) asm("nop");
        *(uint8_t *)buf = (unsigned char)uart_read_byte(UART);
        cnt += 1;
        if (*(uint8_t *)buf == 0x0a) break;
        (uint8_t *)buf++;
    }
    return cnt;
}
