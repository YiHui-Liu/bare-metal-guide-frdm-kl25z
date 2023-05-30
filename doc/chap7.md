## 供应商CMSIS头文件

在前面的部分，我们仅使用数据手册、编辑器和GCC编译器开发了固件程序，使用数据手册创建了外设结构定义。

现在我们已经知道MCU是怎么工作的，是时候介绍一下CMSIS头文件了。它是什么？它是由MCU厂商创建和提供的带有全部定义的头文件。它包含MCU相关的全部，所以很庞大。

CMSIS代表通用微控制器软件接口标准（Common Microcontroller Software Interface Standard），因此它是MCU制造商指定外设API的共同基础。 因为CMSIS是一种ARM标准，并且CMSIS头文件由MCU厂商提供，所以是权威的来源。因此，使用供应商头文件是首选方法，而不是手动编写定义。

在这一节，我们将使用供应商CMSIS头文件替换 `mcu.h` 中的API函数，并保持固件其它部分不变。

STM32 F4系列的CMSIS头文件在这个[仓库](https://github.com/STMicroelectronics/cmsis_device_f4)，从那里将以下文件拷到我们的固件文件夹[step-5-cmsis](step-5-cmsis)：

- [stm32f429xx.h](https://raw.githubusercontent.com/STMicroelectronics/cmsis_device_f4/master/Include/stm32f429xx.h)
- [system_stm32f4xx.h](https://raw.githubusercontent.com/STMicroelectronics/cmsis_device_f4/master/Include/system_stm32f4xx.h)

Those two files depend on a standard ARM CMSIS includes, download them too:
- [core_cm4.h](https://raw.githubusercontent.com/STMicroelectronics/STM32CubeF4/master/Drivers/CMSIS/Core/Include/core_cm4.h)
- [cmsis_gcc.h](https://raw.githubusercontent.com/STMicroelectronics/STM32CubeF4/master/Drivers/CMSIS/Core/Include/cmsis_gcc.h)
- [cmsis_version.h](https://raw.githubusercontent.com/STMicroelectronics/STM32CubeF4/master/Drivers/CMSIS/Core/Include/cmsis_version.h)
- [cmsis_compiler.h](https://raw.githubusercontent.com/STMicroelectronics/STM32CubeF4/master/Drivers/CMSIS/Core/Include/cmsis_compiler.h)
- [mpu_armv7.h](https://raw.githubusercontent.com/STMicroelectronics/STM32CubeF4/master/Drivers/CMSIS/Core/Include/mpu_armv7.h)

然后移除 `mcu.h` 中所有外设API和定义，只留下标准C包含、供应商CMSIS包含，引脚定义等：

```c
#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "stm32f429xx.h"

#define FREQ 16000000  // CPU frequency, 16 Mhz
#define BIT(x) (1UL << (x))
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

static inline void spin(volatile uint32_t count) {
  while (count--) asm("nop");
}

static inline bool timer_expired(uint32_t *t, uint32_t prd, uint32_t now) {
  ...
}
```

如果我们执行 `make clean build` 重新编译固件，GCC会报错：缺少 `systick_init()`, `GPIO_MODE_OUTPUT`, `uart_init()`,
和 `UART3`。我们使用STM32 CMSIS文件重新添加它们。

从 `systick_init()` 开始， `core_cm4.h` 头文件中定义了 `SysTick_Type` 结构体，与我们的 `struct systick` 和 `SysTick` 外设相关宏定义作用相同。还有，`stm32f429xx.h` 头文件中有一个 `RCC_TypeDef` 结构体与我们的 `RCC` 宏定义一样，所以我们的 `systick_init()` 函数几乎不用修改，只需要用 `SYSTICK` 替换 `SysTick`：

```c
static inline void systick_init(uint32_t ticks) {
  if ((ticks - 1) > 0xffffff) return;  // Systick timer is 24 bit
  SysTick->LOAD = ticks - 1;
  SysTick->VAL = 0;
  SysTick->CTRL = BIT(0) | BIT(1) | BIT(2);  // Enable systick
  RCC->APB2ENR |= BIT(14);                   // Enable SYSCFG
}
```

接下来是 `gpio_set_mode()` 函数。`stm32f429xx.h` 头文件中有一个 `GPIO_TypeDef` 结构体，与我们的 `struct gpio` 相同，使用它改写：

```c
#define GPIO(bank) ((GPIO_TypeDef *) (GPIOA_BASE + 0x400 * (bank)))
enum { GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG };

static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
  GPIO_TypeDef *gpio = GPIO(PINBANK(pin));  // GPIO bank
  int n = PINNO(pin);                      // Pin number
  RCC->AHB1ENR |= BIT(PINBANK(pin));       // Enable GPIO clock
  gpio->MODER &= ~(3U << (n * 2));         // Clear existing setting
  gpio->MODER |= (mode & 3) << (n * 2);    // Set new mode
}
```

`gpio_set_af()` 和 `gpio_write()` 也是一样简单替换就行。

然后是串口，CMSIS中有 `USART_TypeDef` 和 `USART1`、`USART2`、`USART3` 的定义，使用它们：

```c
#define UART1 USART1
#define UART2 USART2
#define UART3 USART3
```

在 `uart_init()` 以及其它串口函数中将 `struct uart` 替换为 `USART_TypeDef`，其余部分保持不变。

做完这些，重新编译和烧写固件。LED又闪烁起来，串口也有输出了。恭喜！

我们已经使用供应商CMSIS头文件重写了固件代码，现在重新组织下代码，把所有标准文件放到 `include` 目录下，然后更新Makefile文件让GCC编译器知道：

```make
...
  -g3 -Os -ffunction-sections -fdata-sections -I. -Iinclude \
```

现在得到了一个可以在未来的工程中重用的工程模板。

完整工程源码可以在 [step-5-cmsis](step-5-cmsis) 文件夹找到。
