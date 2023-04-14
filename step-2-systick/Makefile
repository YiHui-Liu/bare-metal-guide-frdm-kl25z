CFLAGS  ?= -W -Wall -Wextra -Werror -Wundef -Wshadow -Wdouble-promotion \
           -Wformat-truncation -fno-common -Wconversion \
           -g3 -Og -ffunction-sections -fdata-sections \
		   -Iinclude -mcpu=cortex-m0plus -mthumb $(EXTRA_CFLAGS)
LDFLAGS ?= -Tscripts/link.ld -nostartfiles -nostdlib --specs nano.specs -lc -lgcc -Wl,--gc-sections -Wl,-Map=build/$@.map
SOURCES = main.c

ifeq ($(OS),Windows_NT)
  RM = cmd /C del /Q /F
else
  RM = rm -f
endif

build: firmware.bin

firmware.elf: $(SOURCES)
	arm-none-eabi-gcc $(SOURCES) $(CFLAGS) $(LDFLAGS) -o build/$@

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary build/$< build/$@

flash: firmware.bin
	JLinkExe -device MKL25Z128XXX4 -if SWD -speed 4000 -autoconnect 1 -CommanderScript scripts/flash.jlink

clean:
	$(RM) build/firmware.*
