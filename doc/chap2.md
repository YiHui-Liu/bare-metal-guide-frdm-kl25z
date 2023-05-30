## 闪烁LED

现在我们已经搭建好了完整的构建、烧写的基础设施，是时候让固件做点儿有用的事情了。什么是有用的事情？当然是闪烁LED了！Nucleo-F429ZI开发板有3颗LED，在开发板数据手册的6.5节，我们可以看到板载LED连接的引脚：

- PB0: green LED
- PB7: blue LED
- PB14: red LED

再次修改 `main.c` 文件，添加上引脚定义，然后把蓝色LED引脚设为输出模式，开始无限循环。首先，把我们之前讨论过的GPIO定义和模式设置拷贝过来，注意，现在又新加了一个 `BIT(position)` 工具宏：

```c
#include <inttypes.h>
#include <stdbool.h>

#define BIT(x) (1UL << (x))
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

struct gpio {
  volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2];
};
#define GPIO(bank) ((struct gpio *) (0x40020000 + 0x400 * (bank)))

// Enum values are per datasheet: 0, 1, 2, 3
enum { GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG };

static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
  struct gpio *gpio = GPIO(PINBANK(pin));  // GPIO bank
  int n = PINNO(pin);                      // Pin number
  gpio->MODER &= ~(3U << (n * 2));         // Clear existing setting
  gpio->MODER |= (mode & 3) << (n * 2);    // Set new mode
}
```

某些微控制器在上电时会把所有外设都自动使能，然而，STM32微控制器在上电时外设是默认关闭的，以降低功耗。为了使能GPIO外设，我们需要通过RCC单元使能外设时钟。在芯片数据手册7.3.10节，可以找到AHB1ENR寄存器与此相关，还是先定义整个RCC单元：

```c
struct rcc {
  volatile uint32_t CR, PLLCFGR, CFGR, CIR, AHB1RSTR, AHB2RSTR, AHB3RSTR,
      RESERVED0, APB1RSTR, APB2RSTR, RESERVED1[2], AHB1ENR, AHB2ENR, AHB3ENR,
      RESERVED2, APB1ENR, APB2ENR, RESERVED3[2], AHB1LPENR, AHB2LPENR,
      AHB3LPENR, RESERVED4, APB1LPENR, APB2LPENR, RESERVED5[2], BDCR, CSR,
      RESERVED6[2], SSCGR, PLLI2SCFGR;
};
#define RCC ((struct rcc *) 0x40023800)
```

在AHB1ENR寄存器文档中可以看到0-8位控制GPIOA - GPIOI的时钟：

```c
int main(void) {
  uint16_t led = PIN('B', 7);            // Blue LED
  RCC->AHB1ENR |= BIT(PINBANK(led));     // Enable GPIO clock for LED
  gpio_set_mode(led, GPIO_MODE_OUTPUT);  // Set blue LED to output mode
  for (;;) asm volatile("nop");          // Infinite loop
  return 0;
}
```

接下来需要做的就是找到如何开关GPIO引脚，然后在主循环中点亮LED，延时，熄灭LED，延时。在芯片数据手册8.4.7节，可以看到BSRR寄存器与设置电压高低有关，低16位设置ODR寄存器输出高，高16位设置ODR寄存器输出低。为此定义一个API函数：

```c
static inline void gpio_write(uint16_t pin, bool val) {
  struct gpio *gpio = GPIO(PINBANK(pin));
  gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
}
```

下一步我们需要实现一个延时函数，目前还不需要精确延时，所以定义一个 `spin()` 函数，执行NOP指令给定的次数：

```c
static inline void spin(volatile uint32_t count) {
  while (count--) asm("nop");
}
```

最后，修改主循环来让LED闪烁起来：

```c
  for (;;) {
    gpio_write(pin, true);
    spin(999999);
    gpio_write(pin, false);
    spin(999999);
  }
```

执行 `make flash` 来看蓝色LED闪烁吧！

完整工程源码可以在 [step-1-blinky](step-1-blinky) 文件夹找到。
