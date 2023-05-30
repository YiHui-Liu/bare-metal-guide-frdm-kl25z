# 准备工作
## 硬件准备
我们将使用[FRDM KL25Z128](https://www.nxp.com/design/development-boards/freedom-development-boards/mcu-boards/freedom-development-platform-for-kinetis-kl14-kl15-kl24-kl25-mcus:FRDM-KL25Z)开发板作为核心板，具体的电路设计为清华大学[Blazar](https://www.blazar.cn)开源硬件系统。

Blazar开源硬件系统以类游戏机的功能布局，搭载了I/O按键、音频、视频接口，具有温度、光线、运动传感器以及标准通讯接口和扩展引脚等，这些模块中包括了I/O、中断、时钟、PWM、通讯、数模变换等嵌入式系统学习的基本单元，可以开发完成各种趣味程序，也可以进一步拓展GPS、蓝牙模块和各种创意应用。

Blazar 项目团队2017年在清华大学制作完成了MOOC在线开放课程 [ARM微控制器与嵌入式系统](https://www.xuetangx.com/course/THU08091000246/14768615?channel=i.area.manual_search)。2018年，此MOOC课程被评为国家精品在线开放课程，累计已有数万人次在线选课学习。

## 工具配置

为继续进行，需要以下工具：

- [ARM GCC](https://developer.arm.com/downloads/-/gnu-rm) - 编译和链接
- [GNU make](http://www.gnu.org/software/make/) - 构建自动化
- [SEGGER J-Link](https://www.segger.com/downloads/jlink/) - 烧写固件与调试

### Mac安装

打开终端，执行：

```sh
$ /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
$ brew install gcc-arm-embedded make
```

前往 [SEGGER J-Link](https://www.segger.com/downloads/jlink/) 下载对应安装包安装好 J-Link。

### Linux(Ubuntu)安装

打开终端，执行：

```sh
$ sudo apt -y install gcc-arm-none-eabi make
```

前往 [SEGGER J-Link](https://www.segger.com/downloads/jlink/) 下载对应安装包安装好 J-Link。

## 数据手册

- [KL25 Sub-Family Reference Manual](https://gab.wallawalla.edu/~larry.aamodt/cptr480/nxp/KL25P80M48SF0RM.pdf)
- [Cortex-M0+ Devices Generic User Guide](https://developer.arm.com/documentation/dui0662/b/)
