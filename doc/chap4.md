## 添加串口调试输出

现在是时候给固件添加一些人类可读的诊断信息了。MCU外设中有一个串行通信接口，通常被称作串口。看一下芯片数据手册2.3节，STM32F429有多个串口控制器，适当配置后就可以通过特定引脚与外部交换数据。最小化的串口配置需要2个引脚，一个接收，另一个发送。

在Nucleo开发板数据手册6.9节，可以看到MCU的串口3的发送引脚是PD8，接收引脚是PD9，并且已经被连到了板载的ST-LINK调试器上，这意味着我们配置好串口3就可以通过PD8发送数据，然后通过ST-LINK在工作站上看到MCU发送的数据。

现在给串口创建API，就像之前GPIO那样。芯片数据手册30.6节概括了串口寄存器，可以这样定义串口结构体：

```c
struct uart {
  volatile uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR;
};
#define UART1 ((struct uart *) 0x40011000)
#define UART2 ((struct uart *) 0x40004400)
#define UART3 ((struct uart *) 0x40004800)
```

要配置串口，需要这些步骤：

- 使能串口时钟，通过设置 `RCC->APB2ENR` 寄存器的相应位
- 设置接收和发送引脚为替代功能，替代功能列表在芯片数据手册表12
- 设置波特率（通信速率），通过 `BRR` 寄存器
- 使能串口外设，通过 `CR1` 寄存器接收和发送数据

我们已经知道如何把GPIO引脚设为特定的模式，如果1个引脚被用作替代功能，我们也必须指定替代功能编号，可以通过GPIO外设的替代功能寄存器 `AFR` 进行控制。仔细阅读芯片数据手册中对 `AFR` 寄存器的描述，可以发现替代功能有4位编号，所以要控制全部16个引脚需要2个32位寄存器。设置引脚替代功能的API可以这样实现：

```c
static inline void gpio_set_af(uint16_t pin, uint8_t af_num) {
  struct gpio *gpio = GPIO(PINBANK(pin));  // GPIO bank
  int n = PINNO(pin);                      // Pin number
  gpio->AFR[n >> 3] &= ~(15UL << ((n & 7) * 4));
  gpio->AFR[n >> 3] |= ((uint32_t) af_num) << ((n & 7) * 4);
}
```

为了从GPIO API中完全隐藏寄存器特定的代码，我们把GPIO时钟初始化的代码移动到 `gpio_set_mode()` 函数中：

```c
static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
  struct gpio *gpio = GPIO(PINBANK(pin));  // GPIO bank
  int n = PINNO(pin);                      // Pin number
  RCC->AHB1ENR |= BIT(PINBANK(pin));       // Enable GPIO clock
  ...
```

现在可以创建一个串口初始化的API函数：

```c
#define FREQ 16000000  // CPU frequency, 16 Mhz
static inline void uart_init(struct uart *uart, unsigned long baud) {
  // https://www.st.com/resource/en/datasheet/stm32f429zi.pdf
  uint8_t af = 7;           // Alternate function
  uint16_t rx = 0, tx = 0;  // pins

  if (uart == UART1) RCC->APB2ENR |= BIT(4);
  if (uart == UART2) RCC->APB1ENR |= BIT(17);
  if (uart == UART3) RCC->APB1ENR |= BIT(18);

  if (uart == UART1) tx = PIN('A', 9), rx = PIN('A', 10);
  if (uart == UART2) tx = PIN('A', 2), rx = PIN('A', 3);
  if (uart == UART3) tx = PIN('D', 8), rx = PIN('D', 9);

  gpio_set_mode(tx, GPIO_MODE_AF);
  gpio_set_af(tx, af);
  gpio_set_mode(rx, GPIO_MODE_AF);
  gpio_set_af(rx, af);
  uart->CR1 = 0;                           // Disable this UART
  uart->BRR = FREQ / baud;                 // FREQ is a UART bus frequency
  uart->CR1 |= BIT(13) | BIT(2) | BIT(3);  // Set UE, RE, TE
}
```

最后，再来实现串口读写函数。芯片数据手册30.6.1节告诉我们状态寄存器 `SR` 表示数据是否准备好：

```c
static inline int uart_read_ready(struct uart *uart) {
  return uart->SR & BIT(5);  // If RXNE bit is set, data is ready
}
```

数据可以从数据寄存器 `DR` 中获取：

```c
static inline uint8_t uart_read_byte(struct uart *uart) {
  return (uint8_t) (uart->DR & 255);
}
```

发送单个字节的数据也是通过 `DR` 寄存器完成。设置好要发送的数据后，我们需要等待发送完成，通过检查 `SR` 寄存器第7位来实现：

```c
static inline void uart_write_byte(struct uart *uart, uint8_t byte) {
  uart->DR = byte;
  while ((uart->SR & BIT(7)) == 0) spin(1);
}
```

写数据到缓冲区：

```c
static inline void uart_write_buf(struct uart *uart, char *buf, size_t len) {
  while (len-- > 0) uart_write_byte(uart, *(uint8_t *) buf++);
}
```

在 `main()` 函数中初始化串口：

```c
  ...
  uart_init(UART3, 115200);              // Initialise UART
```

然后每次闪烁LED时输出一条消息 "hi\r\n"：

```c
    if (timer_expired(&timer, period, s_ticks)) {
      ...
      uart_write_buf(UART3, "hi\r\n", 4);  // Write message
    }
```

重新编译，然后烧写到开发板上，用一个终端程序连接ST-LINK的端口。在Mac上，我用 `cu`，在Linux上也可以用它。在Windows上使用 `putty` 工具是一个好主意。打开终端，执行命令后可以看到：

```sh
$ cu -l /dev/cu.YOUR_SERIAL_PORT -s 115200
hi
hi
```

完整工程代码可以在 [step-3-uart](step-3-uart) 文件夹找到。