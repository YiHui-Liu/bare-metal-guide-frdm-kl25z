# 裸机编程指南

[![License: MIT](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)
[![Build Status]( https://github.com/cpq/bare-metal-programming-guide/workflows/build/badge.svg)](https://github.com/cpq/bare-metal-programming-guide/actions)

本指南是为那些希望用GCC编译器和数据手册而无需其他任何东西就能开始为微控制器（单片机）编程的开发者而写的。本指南中的基础知识可以帮助你更好地理解像STM32Cube、Keil、Arduino和其他框架或IDE是怎么工作的。

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

* [准备工作](doc/chap0.md)
* [微控制器介绍](doc/chap1.md)
* [闪烁LED](doc/chap2.md)
* [用SysTick中断实现闪烁](doc/chap3.md)
* [添加串口调试输出](doc/chap4.md)
* [重定向`printf()`到串口](doc/chap5.md)
* [调试](doc/chap6.md)
* [供应商CMSIS头文件](doc/chap7.md)
