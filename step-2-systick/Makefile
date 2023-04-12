CFLAGS  ?= -mcpu=cortex-m0plus -Iinclude $(EXTRA_CFLAGS)
LDFLAGS ?= -Tlink.ld -nostartfiles -nostdlib --specs nano.specs -lc -lgcc -Wl,--gc-sections -Wl,-Map=$@.map
SOURCES = main.c

build: firmware.bin

firmware.elf: $(SOURCES)
	arm-none-eabi-gcc $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $@

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

flash: firmware.bin
	JLinkExe -device MKL25Z128XXX4 -if SWD -speed 4000 -autoconnect 1 -CommanderScript flash.jlink

clean:
	rm -rf firmware.*
