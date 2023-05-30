# 裸机编程指南

[![License: MIT](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)
[![Build Status]( https://github.com/cpq/bare-metal-programming-guide/workflows/build/badge.svg)](https://github.com/cpq/bare-metal-programming-guide/actions)

本指南是为那些希望用GCC编译器和数据手册而无需其他任何东西就能开始为微控制器（单片机）编程的开发者而写的。本指南中的基础知识可以帮助你更好地理解像STM32Cube、Keil、Arduino和其他框架或IDE是怎么工作的。

## 内容
本指南由若干章节组成，每个章节都有一个相关的完整小项目可以实战，涵盖了以下话题：

- 存储和寄存器
- 中断向量表
- 启动代码
- 链接脚本
- 使用 `make` 进行自动化构建
- GPIO外设和闪烁LED
- SysTick定时器
- UART外设和调试输出
- `printf`重定向到UART
- 用J-Link进行调试
- PWM和蜂鸣器

## 目录
* [准备工作](doc/chap0.md)
* [微控制器介绍](doc/chap1.md)
* [闪烁LED](doc/chap2.md)
* [用SysTick中断实现闪烁](doc/chap3.md)
* [添加串口调试输出](doc/chap4.md)
* [重定向`printf()`到串口](doc/chap5.md)
* [调试](doc/chap6.md)
* [供应商CMSIS头文件](doc/chap7.md)

## 资源链接
### 工具
- [ARM GCC](https://developer.arm.com/downloads/-/gnu-rm) - 编译和链接
- [GNU make](http://www.gnu.org/software/make/) - 构建自动化
- [SEGGER J-Link](https://www.segger.com/downloads/jlink/) - 烧写固件与调试

### 数据手册
- [KL25 Sub-Family Reference Manual](https://gab.wallawalla.edu/~larry.aamodt/cptr480/nxp/KL25P80M48SF0RM.pdf)
- [Cortex-M0+ Devices Generic User Guide](https://developer.arm.com/documentation/dui0662/b/)

### 课程
- [ARM微控制器与嵌入式系统](https://www.xuetangx.com/course/THU08091000246/14768615?channel=i.area.manual_search)
