#include "derivative.h"
#include <string.h>

extern int main(void);  // Defined in main.c

static void __init_hardware() {
    // Switch off watchdog
    SIM->COPC = 0x0;
}

static void zero_fill_bss(void) {
    extern char _sbss[];
    extern char _ebss[];
    memset(_sbss, 0, (size_t)(_ebss - _sbss));
}

static void copy_data(void) {
    extern char _sdata[];
    extern char _edata[];
    extern char _sidata[];
    memcpy(_sdata, _sidata, (size_t)(_edata - _sdata));
}

static uint32_t MCGOUTClock;
static uint16_t Divider;

static void FLL_clock(void) {
    /*
        FRDIV: FLL External Reference Divider, bits 3-5 of MCG_C1
        0b000 - 0b101: Divide Factor is 2^FRDIV
        110: Divide Factor is 1280
        111: Divide Factor is 1536

        IREFS: Internal Reference Select, bit 2 of MCG_C1
        0: External reference clock is selected.
        1: The slow internal reference clock is selected.

        RANGE0: Frequency Range Select, bit 4-5 of MCG_C2
        00: Low frequency range selected for the crystal oscillator.
        01: High frequency range selected for the crystal oscillator.
        1x: Very high frequency range selected for the crystal oscillator.
    */
    // FLL reference clock
    if ((MCG->C1 & MCG_C1_IREFS_MASK) == 0x00U) {
        if ((MCG->C2 & MCG_C2_RANGE0_MASK) != 0x00U) switch (MCG->C1 & MCG_C1_FRDIV_MASK) {
                case 0x38U:
                    Divider = 1536U;
                    break;
                case 0x30U:
                    Divider = 1280U;
                    break;
                default:
                    Divider = (uint16_t)(32LU << ((MCG->C1 & MCG_C1_FRDIV_MASK) >> MCG_C1_FRDIV_SHIFT));
                    break;
            }
        else
            Divider = (uint16_t)(1LU << ((MCG->C1 & MCG_C1_FRDIV_MASK) >> MCG_C1_FRDIV_SHIFT));

        MCGOUTClock = (CPU_XTAL_CLK_HZ / Divider);
    } else
        MCGOUTClock = CPU_INT_SLOW_CLK_HZ;

    /*
        DMX32: DCO Maximum Frequency with 32.768 kHz Reference, bit 7 of MCG_C4
        DRST_DRS: DCO Range Select, bits 5-6 of MCG_C4
    */
    switch (MCG->C4 & (MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS_MASK)) {
        case 0x00U:
            MCGOUTClock *= 640U;
            break;
        case 0x20U:
            MCGOUTClock *= 1280U;
            break;
        case 0x40U:
            MCGOUTClock *= 1920U;
            break;
        case 0x60U:
            MCGOUTClock *= 2560U;
            break;
        case 0x80U:
            MCGOUTClock *= 732U;
            break;
        case 0xA0U:
            MCGOUTClock *= 1464U;
            break;
        case 0xC0U:
            MCGOUTClock *= 2197U;
            break;
        case 0xE0U:
            MCGOUTClock *= 2929U;
            break;
        default:
            break;
    }
}

static void PLL_clock(void) {
    /*
        PRDIV0: PLL External Reference Divider, bits 0-4 of MCG_C5
        value from 00000 to 11000, divide Factor from 1 to 25

        VDIV0: VCO 0 Divider, bits 0-4 of MCG_C6
        value from 00000 to 11111, multiply Factor from 24 to 55
    */
    Divider = (((uint16_t)MCG->C5 & MCG_C5_PRDIV0_MASK) + 1U);
    MCGOUTClock = (uint32_t)(CPU_XTAL_CLK_HZ / Divider);
    Divider = (((uint16_t)MCG->C6 & MCG_C6_VDIV0_MASK) + 24U);
    MCGOUTClock *= Divider;
}

static void inter_clock(void) {
    /*
        IRCS: Internal Reference Clock Select, bit 0 of MCG_C2
        0: Slow internal reference clock selected.
        1: Fast internal reference clock selected.

        FCRDIV: Fast Clock Internal Reference Divider, bits 1-3 of MCG_SC
        0b000 - 0b111: Divide Factor is 2^FCRDIV
    */
    if ((MCG->C2 & MCG_C2_IRCS_MASK) == 0x00U)
        MCGOUTClock = CPU_INT_SLOW_CLK_HZ;
    else {
        Divider = (uint16_t)(0x01LU << ((MCG->SC & MCG_SC_FCRDIV_MASK) >> MCG_SC_FCRDIV_SHIFT));
        MCGOUTClock = (uint32_t)(CPU_INT_FAST_CLK_HZ / Divider);
    }
}

static void exter_clock(void) { MCGOUTClock = CPU_XTAL_CLK_HZ; }

static void clock_init(void) {
    /*
        CLKS: Clock Source Select, bits 6-7 of MCG_C1
        00: Output of FLL or PLL is selected (depends on PLLS control bit).
        01: Internal reference clock is selected.
        10: External reference clock is selected.

        PLLS: PLL Select, bit 4 of MCG_C6
        0: FLL is selected.
        1: PLL is selected. (PRDIV0 must be programmed so divider can generate a PLL reference clock)

        OUTDIV1: Clock 1 Output Divider, bits 28-31 of SIM_CLKDIV1
        value from 0b0000 to 0b1111, divide factor from 1 to 16

        OUTDIV4: Clock 4 Output Divider, bits 16-18 of SIM_CLKDIV1
        value from 0b000 to 0b111, divide factor from 1 to 8, default 0b001 (2)
    */
    if ((MCG->C1 & MCG_C1_CLKS_MASK) == 0x00U) {
        if ((MCG->C6 & MCG_C6_PLLS_MASK) == 0x00U)
            FLL_clock();
        else
            PLL_clock();
    } else if ((MCG->C1 & MCG_C1_CLKS_MASK) == 0x40U)
        inter_clock();
    else if ((MCG->C1 & MCG_C1_CLKS_MASK) == 0x80U)
        exter_clock();

    CORCLK = (MCGOUTClock / (0x01U + ((SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV1_MASK) >> SIM_CLKDIV1_OUTDIV1_SHIFT)));
    CORCLK = (uint32_t)(CORCLK / 1000U) * 1000U;
    BUSCLK = CORCLK / (0x01U + ((SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV4_MASK) >> SIM_CLKDIV1_OUTDIV4_SHIFT));
    BUSCLK = (uint32_t)(BUSCLK / 1000U) * 1000U;
}

__attribute__((naked, noreturn)) void _reset(void) {
    __init_hardware();
    zero_fill_bss();
    copy_data();
    clock_init();

    main();
    for (;;) (void)0;
}

// Interrupt service routine (ISR)
extern void _estack(void);  // Defined in link.ld

void Default_Handler() { __asm("bkpt"); }
void NMI_Handler() __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler() __attribute__((weak, alias("Default_Handler")));
void SVC_Handler() __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler() __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler() __attribute__((weak, alias("Default_Handler")));

// Interrupt request (IRQ) Handlers
void DMA0_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void DMA1_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void DMA2_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void DMA3_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void MCM_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void FTFL_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void PMC_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void LLWU_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void I2C0_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void I2C1_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void SPI0_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void UART0_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void UART1_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void UART2_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void ADC0_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void CMP0_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void TPM0_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void TPM1_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void TPM2_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void RTC_Seconds_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void PIT_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void USBOTG_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void DAC0_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void TSI0_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void MCG_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void LPTMR0_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void PORTA_IRQHandler() __attribute__((weak, alias("Default_Handler")));
void PORTD_IRQHandler() __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".vectors"))) void (*tab[16 + 32])(void) = {
    _estack,                 // Initial stack pointer
    _reset,                  // Reset handler
    NMI_Handler,             // NMI handler
    HardFault_Handler,       // Hard Fault handler
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    SVC_Handler,             // SVC handler
    0,                       // Reserved
    0,                       // Reserved
    PendSV_Handler,          // PendSV handler
    SysTick_Handler,         // SysTick handler
    DMA0_IRQHandler,         // DMA channel 0 transfer complete and error
    DMA1_IRQHandler,         // DMA channel 1 transfer complete and error
    DMA2_IRQHandler,         // DMA channel 2 transfer complete and error
    DMA3_IRQHandler,         // DMA channel 3 transfer complete and error
    MCM_IRQHandler,          // Normal interrupt
    FTFL_IRQHandler,         // command complete and read collision
    PMC_IRQHandler,          // Low-voltage detect, low-voltage warning
    LLWU_IRQHandler,         // Low Leakage Wakeup
    I2C0_IRQHandler,         // I2C0 interrupt
    I2C1_IRQHandler,         // I2C1 interrupt
    SPI0_IRQHandler,         // SPI0 interrupt
    SPI1_IRQHandler,         // SPI1 interrupt
    UART0_IRQHandler,        // UART0 status and error
    UART1_IRQHandler,        // UART1 status and error
    UART2_IRQHandler,        // UART2 status and error
    ADC0_IRQHandler,         // ADC0 interrupt
    CMP0_IRQHandler,         // CMP0 interrupt
    TPM0_IRQHandler,         // TPM0 fault, overflow and channels interrupt
    TPM1_IRQHandler,         // TPM1 fault, overflow and channels interrupt
    TPM2_IRQHandler,         // TPM2 fault, overflow and channels interrupt
    RTC_Alarm_IRQHandler,    // RTC alarm interrupt
    RTC_Seconds_IRQHandler,  // RTC seconds interrupt
    PIT_IRQHandler,          // PIT timer all channels interrupt
    Default_Handler,         // Reserved
    USBOTG_IRQHandler,       // USB OTG interrupt
    DAC0_IRQHandler,         // DAC0 interrupt
    TSI0_IRQHandler,         // TSI0 interrupt
    MCG_IRQHandler,          // MCG interrupt
    LPTMR0_IRQHandler,       // LPTimer interrupt
    Default_Handler,         // Reserved
    PORTA_IRQHandler,        // Port A interrupt
    PORTD_IRQHandler,        // Port D interrupt
};

__attribute__((section(".cfmconfig"))) uint32_t(cfm[4]) = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE};
