## 重定向`printf()`到串口

在这一节，我们将 `uart_write_buf()` 调用替换为 `printf()`，它使我们能够进行格式化输出，这样可以更好的输出诊断信息，实现了“打印样式的调试”。

我们使用的GNU ARM工具链除了包含GCC编译器和一些工具外，还包含了一个被称为[newlib](https://sourceware.org/newlib)的C库，由红帽为嵌入式系统开发。

如果我们的固件调用了一个标准C库函数，比如 `strcmp()`，newlib就会被GCC链接器加到我们的固件中。

newlib实现了一些标准C函数，特别是文件输入输出操作，并且被实现的很随潮流：这些函数最终调用一组被称为 "syscalls" 的底层输入输出函数。

例如：

- `fopen()` 最终调用 `_open()`
- `fread()` 最终调用 `_read()`
- `fwrite()`, `fprintf()`, `printf()` 最终调用 `_write()`
- `malloc` 最终调用 `_sbrk()`，等等

因此，通过修改 `_write()` 系统调用，我们可以重定向 `printf()` 到任何我们希望的地方，这个机制被称为 "IO retargeting"。

注意，STM32 Cube也使用ARM GCC工具链，这就是为什么Cube工程都包含 `syscalls.c` 文件。其它工具链，比如TI的CCS、Keil的CC，可能使用不同的C库，重定向机制会有一点区别。我们用newlib，所以修改 `_write()` 可以打印到串口3。

在那之前，我们先重新组织下源码结构：

- 把所有API定义放到 `mcu.h` 文件中
- 把启动代码放到 `startup.c` 文件中
- 为newlib的系统调用创建一个空文件 `syscalls.c`
- 修改Makefile，把 `syscalls.c` 和 `startup.c` 加到build中

将所有 API 定义移动到 `mcu.h` 后，`main.c` 文件变得相当紧凑。注意我们还没提到底层寄存器，高级API函数很容易理解：

```c
#include "mcu.h"

static volatile uint32_t s_ticks;
void SysTick_Handler(void) {
  s_ticks++;
}

int main(void) {
  uint16_t led = PIN('B', 7);            // Blue LED
  systick_init(16000000 / 1000);         // Tick every 1 ms
  gpio_set_mode(led, GPIO_MODE_OUTPUT);  // Set blue LED to output mode
  uart_init(UART3, 115200);              // Initialise UART
  uint32_t timer = 0, period = 500;      // Declare timer and 500ms period
  for (;;) {
    if (timer_expired(&timer, period, s_ticks)) {
      static bool on;                      // This block is executed
      gpio_write(led, on);                 // Every `period` milliseconds
      on = !on;                            // Toggle LED state
      uart_write_buf(UART3, "hi\r\n", 4);  // Write message
    }
    // Here we could perform other activities!
  }
  return 0;
}
```

现在我们把 `printf()` 重定向到串口3，在空的 `syscalls.c` 文件中拷入一下内容：

```c
#include "mcu.h"

int _write(int fd, char *ptr, int len) {
  (void) fd, (void) ptr, (void) len;
  if (fd == 1) uart_write_buf(UART3, ptr, (size_t) len);
  return -1;
}
```

这段代码：如果我们写入的文件描述符是 1（这是一个标准输出描述符），则将缓冲区写入串口3，否则忽视。这就是重定向的本质！

重新编译，会得到一些链接器错误：

```sh
../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-sbrkr.o): in function `_sbrk_r':
sbrkr.c:(.text._sbrk_r+0xc): undefined reference to `_sbrk'
closer.c:(.text._close_r+0xc): undefined reference to `_close'
lseekr.c:(.text._lseek_r+0x10): undefined reference to `_lseek'
readr.c:(.text._read_r+0x10): undefined reference to `_read'
fstatr.c:(.text._fstat_r+0xe): undefined reference to `_fstat'
isattyr.c:(.text._isatty_r+0xc): undefined reference to `_isatty'
```

这是因为我们使用了newlib的标准输入输出函数，那么就需要把newlib中其它的系统调用也实现。加入一些简单的什么都不做的桩函数：

```c
int _fstat(int fd, struct stat *st) {
  (void) fd, (void) st;
  return -1;
}

void *_sbrk(int incr) {
  (void) incr;
  return NULL;
}

int _close(int fd) {
  (void) fd;
  return -1;
}

int _isatty(int fd) {
  (void) fd;
  return 1;
}

int _read(int fd, char *ptr, int len) {
  (void) fd, (void) ptr, (void) len;
  return -1;
}

int _lseek(int fd, int ptr, int dir) {
  (void) fd, (void) ptr, (void) dir;
  return 0;
}
```

再重新编译，应该就不会报错了。

最后一步，将 `main()` 中 `uart_write_buf()` 替换为 `printf()`，并打印一些有用的信息，比如LED状态和当前s_ticks的值：

```c
printf("LED: %d, tick: %lu\r\n", on, s_ticks);  // Write message
```

再重新编译，串口输出应该像这样：

```sh
LED: 1, tick: 250
LED: 0, tick: 500
LED: 1, tick: 750
LED: 0, tick: 1000
```

可喜可贺！我们学习了IO重定向是如何工作的，并且可以用打印输出来调试固件了。

完整工程源码可以在 [step-4-printf](step-4-printf) 文件夹找到。