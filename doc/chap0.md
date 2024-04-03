# 前言
现代编程有多种形式，例如 Web 开发、桌面应用程序开发、移动开发等等，其中白包括嵌入式开发。嵌入式开发是指对旨在嵌入到更大系统中的计算机进行编程，嵌入式计算机通常负责特定任务，而不是通用计算设备。

嵌入式计算机可能运行一个完整的操作系统，或者一个只提供一些实时功能调度的简约系统。在根本没有操作系统的情况下，计算机被称为裸机（bare metal），因此裸机编程是直接为缺少操作系统的（微型）计算机编程。裸机编程可能极具挑战性，并且与其他类型的编程有很大不同，代码直接与底层硬件接口相关，使用电压表和示波器等工具来调试软件的情况并不少见。

现代嵌入式硬件有多种类型，但绝大部分都包含 ARM 架构的 CPU，移动设备通常使用的高通、麒麟或苹果 A 系列 CPU 都基于 ARM 架构。本指南是 ARM 系统的 C 语言裸机编程。具体来说，使用的是 ARM Cortex-M0 架构，这是 32 位 ARM 架构.

## 硬件准备
我们将使用[FRDM KL25Z128](https://www.nxp.com/design/development-boards/freedom-development-boards/mcu-boards/freedom-development-platform-for-kinetis-kl14-kl15-kl24-kl25-mcus:FRDM-KL25Z)开发板作为核心板，具体的电路设计为清华大学[Blazar](https://www.blazar.cn)开源硬件系统。

Blazar 开源硬件系统以类游戏机的功能布局，搭载了 I/O 按键、音频、视频接口，具有温度、光线、运动传感器以及标准通讯接口和扩展引脚等，这些模块中包括了 I/O、中断、时钟、PWM、通讯、数模变换等嵌入式系统学习的基本单元，可以开发完成各种趣味程序，也可以进一步拓展 GPS、蓝牙模块和各种创意应用。

Blazar 项目团队 2017 年在清华大学制作完成了 MOOC 在线开放课程 [ARM微控制器与嵌入式系统](https://www.xuetangx.com/course/THU08091000246/14768615?channel=i.area.manual_search)。2018 年，此 MOOC 课程被评为国家精品在线开放课程，累计已有数万人次在线选课学习。

## Linux 环境准备
本指南的开发将会在 Linux 环境下进行，因此需要准备好 Linux 开发环境。如果您还没有 Linux 开发环境，有三种主要方式可以选择：
1. 在本地安装 Linux：有一台运行 Linux 的计算机，可以直接安装开发环境。
2. 在虚拟机中安装 Linux：在 VMware、VirtualBox 或其他虚拟机软件中安装 Linux 系统。
3. 在 Windows 系统上安装 Linux 子系统（Windows Subsystem for Linux，WSL）：Windows 10 版本 1903 及以上版本支持安装 WSL2 系统。

### 本地安装 Linux（待补充）

### 虚拟机安装 Linux（待补充）

### WSL 安装 Linux（待补充）
安装结束后，由于调试过程中需要用到 USB 接口进行通信，我们需要将 USB 接口从 Windows 系统中转接到 WSL 中，使用的工具为 [usbipd-win](https://github.com/dorssel/usbipd-win)，具体可参考 [连接 USB 设备](https://learn.microsoft.com/zh-cn/windows/wsl/connect-usb)。

## 工具配置
为继续进行，需要以下工具：
- [ARM GCC](https://developer.arm.com/downloads/-/gnu-rm) - 编译和链接
- [GNU make](http://www.gnu.org/software/make/) - 构建自动化
- [SEGGER J-Link](https://www.segger.com/downloads/jlink/) - 烧写固件与调试
- [Visual Studio Code](https://code.visualstudio.com/) - 代码编写与调试

### GCC交叉编译工具链
什么是交叉编译器？它在一个平台上运行，但为另一个平台创建可执行文件，在本指南中通常在 x86-64 平台上运行 Linux，得到 ARM 平台的可执行文件。GNU 构建工具，以及扩展的 GCC，使用目标三元组的概念来描述平台，三元组列出了平台的体系结构、供应商和操作系统或二进制接口类型。目标三元组的供应商部分通常是无关紧要的。您可以通过运行 `gcc -dumpmachine`来查找您自己机器的目标三元组，例如：
```bash
$ gcc -dumpmachine
x86_64-linux-gnu
```

要为 ARM 编译需要选择正确的交叉编译器工具链，即目标三元组与实际目标相匹配的工具链。相当普遍的 `gcc-arm-linux-gnueabi` 工具链无法满足需求，因为该名称表明该工具链旨在为运行 Linux 的 ARM 设备编译代码。本指南进行裸机编程，因此目标系统上没有运行 Linux，需要的工具链目标三元组应该为 `gcc-arm-none-eabi`。在 Ubuntu 上能够简单地安装工具链：
```bash
$ sudo apt-get install gcc-arm-none-eabi
```

此外也可以从 [ARM GCC](https://developer.arm.com/downloads/-/gnu-rm) 下载对应的软件包，安装完成之后将安装位置加入到环境变量`PATH`中。

### 自动化构建工具 make
在 Ubuntu 中打开终端，执行：
```bash
$ sudo apt -y install make
```

在 MacOS 中打开终端，执行：
```bash
$ /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
$ brew install make
```

### 烧录与调试工具 J-Link
前往 [SEGGER J-Link](https://www.segger.com/downloads/jlink/) 下载对应安装包安装好 J-Link，安装完成之后将安装位置加入到环境变量`PATH`中。

### 代码编辑与调试平台 VSCode
安装好 VSCode 本体之后，我们需要安装若干插件完善开发的体验：
* [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools): C/C++ IntelliSense, debugging, and code browsing.
* [Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug): ARM Cortex-M GDB Debugger support for VSCode
* [MemoryView](https://marketplace.visualstudio.com/items?itemName=mcu-debug.memory-view): Provide memory views for debuggers
* [LinkerScript](https://marketplace.visualstudio.com/items?itemName=ZixuanWang.linkerscript): Language support for GNU linker script
* [Arm Assembly](https://marketplace.visualstudio.com/items?itemName=dan-c-underwood.arm): Arm assembly syntax support for Visual Studio Code

## 数据手册
- [KL25 Sub-Family Reference Manual](https://gab.wallawalla.edu/~larry.aamodt/cptr480/nxp/KL25P80M48SF0RM.pdf)
- [Cortex-M0+ Devices Generic User Guide](https://developer.arm.com/documentation/dui0662/b/)

## 参考资料
1. [Visual Studio Code for C/C++ with ARM Cortex-M: Part 1 – Installation](https://mcuoneclipse.com/2021/05/01/visual-studio-code-for-c-c-with-arm-cortex-m-part-1/)
2. [连接 USB 设备](https://learn.microsoft.com/zh-cn/windows/wsl/connect-usb)