## 用Segger Ozone进行调试

如果我们的固件卡在某个地方并且 printf 调试不起作用怎么办？甚至连启动代码都不起作用怎么办？我们需要一个调试器。那有很多选项，但我建议使用Segger的Ozone调试器。为什么？因为它是独立的，不依赖任何IDE。我们可以把 `firmware.elf` 直接提供给Ozone，它会自动拾取源文件。

可以从Segger网站[下载 Ozone](https://www.segger.com/products/development-tools/ozone-j-link-debugger/)。在用它调试我们的Nucleo开发板之前，我们需要把板载的ST-LINK固件改成jlink的固件，这样Ozone才能识别。遵循Segger网站的[说明](https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/)完成固件修改。

现在，运行Ozone，在向导中选择设备：

<img src="../images/ozone1.png" width="50%" />

选择我们要用的调试器硬件：

<img src="../images/ozone2.png" width="50%" />

然后选择 `firmware.elf` 固件文件：

<img src="../images/ozone3.png" width="50%" />

接下来的步骤保持默认，点击“完成”，调试器已经载入（可以看到`mcu.h`源码被拾取）：

<img src="../images/ozone3.png" width="50%" />

点击左上角的绿色按钮，下载、运行固件，然后会停在这里：

![](images/ozone5.png)

现在我们可以单步运行代码，设置断点，以及其它调试工作。有一个地方可以注意，那就是Ozone方便的外设视图：

![](images/ozone6.png)

我们可以用它直接检查或设置外设的状态，例如，点亮板子上的绿色LED（PB0）：

1. 先使能GPIOB时钟，找到  Peripherals -> RCC -> AHB1ENR，然后把 GPIOBEN 位设为1：
  <img src="../images/ozone7.png" width="75%" />

2. 找到 Peripherals -> GPIO -> GPIOB -> MODER，设置 MODER0 为1（输出）：
  <img src="../images/ozone8.png" width="75%" />

3. 找到 Peripherals -> GPIO -> GPIOB -> ODR，设置 ODR0 为1（高电平）：
  <img src="../images/ozone9.png" width="75%" />

这样绿色LED就被点亮了。愉快地调试吧！