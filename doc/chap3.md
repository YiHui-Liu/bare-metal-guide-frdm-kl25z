## 用SysTick中断实现闪烁

为了实现精确的时间控制，我们应该使能ARM的SysTick中断。SysTick是一个24位的硬件计数器，是ARM核的一部分，因为在ARM的数据手册中有它的文档。从芯片数据手册中可以看到，SysTick有4个寄存器：

- CTRL，使能/禁能SysTick
- LOAD，初始计数值
- VAL，当前计数值，每个时钟周期递减
- CALIB，校准寄存器

每次VAL减到0，就会产生一个SysTick中断，SysTick中断在向量表中的索引为15，我们需要设置它。在启动时，Nucleo-F429ZI的时钟是16MHz，我们可以配置SysTick计数器使其每毫秒产生一个中断。

首先，定义SysTick外设，我们知道有4个寄存器，并且从芯片数据手册中可以知道SysTick地址为0xe000e010，编写代码：

```c
struct systick {
  volatile uint32_t CTRL, LOAD, VAL, CALIB;
};
#define SYSTICK ((struct systick *) 0xe000e010)
```

接下来定义一个API函数配置SysTick，我们需要在 `SYSTICK->CTRL` 寄存器使能SysTick，同时也需要在 `RCC->APB2ENR` 中使能它的时钟，这一点在芯片数据手册7.4.14节描述：

```c
#define BIT(x) (1UL << (x))
static inline void systick_init(uint32_t ticks) {
  if ((ticks - 1) > 0xffffff) return;  // Systick timer is 24 bit
  SYSTICK->LOAD = ticks - 1;
  SYSTICK->VAL = 0;
  SYSTICK->CTRL = BIT(0) | BIT(1) | BIT(2);  // Enable systick
  RCC->APB2ENR |= BIT(14);                   // Enable SYSCFG
}
```

默认情况下，Nucleo-F429ZI运行频率为16MHz，这表示如果我们调用 `systick_init(16000000 / 1000)`，就会每毫秒产生一个SysTick中断。还需要定义一个中断处理函数，现在只是在中断处理函数中递增一个32位数：

```c
static volatile uint32_t s_ticks; // volatile is important!!
void SysTick_Handler(void) {
  s_ticks++;
}
```

注意 `s_ticks` 要用 `volatile` 说明符，在中断处理函数中的任何变量都应该标记为 `volatile`，这样可以避免编译器通过在寄存器中缓存变量值的优化操作。`volatile` 关键字可以是生成的代码总是从存储空间载入变量值。

现在把SysTick中断处理函数加到向量表中：

```c
__attribute__((section(".vectors"))) void (*tab[16 + 91])(void) = {
    0, _reset, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SysTick_Handler};
```

这样我们就有精确的毫秒时钟了！再创建一个任意周期的定时器工具函数：

```c
// t: expiration time, prd: period, now: current time. Return true if expired
bool timer_expired(uint32_t *t, uint64_t prd, uint64_t now) {
  if (now + prd < *t) *t = 0;                    // Time wrapped? Reset timer
  if (*t == 0) *t = now + prd;                   // First poll? Set expiration
  if (*t > now) return false;                    // Not expired yet, return
  *t = (now - *t) > prd ? now + prd : *t + prd;  // Next expiration time
  return true;                                   // Expired, return true
}
```

使用这个精确的定时器函数更新闪烁LED的主循环，例如，每500ms闪烁一次：

```c
  uint32_t timer = 0, period = 500;       // Declare timer and 500ms period
  for (;;) {
    if (timer_expired(&timer, period, s_ticks)) {
      static bool on;       // This block is executed
      gpio_write(led, on);  // Every `period` milliseconds
      on = !on;             // Toggle LED state
    }
    // Here we could perform other activities!
  }
```

通过使用 `SysTick` 和 `timer_expired()` 工具函数，使主循环成为非阻塞的。这意味着在主循环中我们还可以执行许多动作，例如，创建不同周期的定时器，它们都能及时被触发。

完整工程源码可以在 [step-2-systick](step-2-systick) 文件夹找到。