# 目录定义
DEPS_DIR = scripts
BUILD_DIR = build

# 变量定义
TARGET = firmware
SOURCES = main.c

# 编译选项
CFLAGS  ?= -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
           -Wformat-truncation -fno-common -Wconversion \
           -g3 -Og -ffunction-sections -fdata-sections \
		   -Iinclude -mcpu=cortex-m0plus -mthumb -lm $(EXTRA_CFLAGS)
LDFLAGS ?= -T $(DEPS_DIR)/link.ld -nostartfiles -nostdlib --specs nano.specs  \
		   -lc -lgcc -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).$@.map

# 命令定义
ifeq ($(OS),Windows_NT)
  RM = cmd /C del /Q /F
else
  RM = rm -f
endif

# 目录创建
$(shell if [ ! -e $(DEPS_DIR) ];then mkdir -p $(DEPS_DIR); fi)
$(shell if [ ! -e $(BUILD_DIR) ];then mkdir -p $(BUILD_DIR); fi)

build: bin

elf: $(SOURCES)
	arm-none-eabi-gcc $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $(BUILD_DIR)/$(TARGET).elf

bin: elf
	arm-none-eabi-objcopy -O binary $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

flash: bin
	JLinkExe -device MKL25Z128XXX4 -if SWD -speed 4000 -autoconnect 1 -CommanderScript $(DEPS_DIR)/flash.jlink

disassembly: elf
	arm-none-eabi-objdump -D -S $(BUILD_DIR)/$(TARGET).elf > $(BUILD_DIR)/$(TARGET).S

disassembly-none: elf
	arm-none-eabi-objdump -D $(BUILD_DIR)/$(TARGET).elf > $(BUILD_DIR)/$(TARGET).S

clean:
	$(RM) $(BUILD_DIR)/$(TARGET).*
