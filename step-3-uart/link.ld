ENTRY(_reset);
MEMORY {
    interrupts(rx) : ORIGIN = 0x00000000, LENGTH = 0xC0
    flash(rx)      : ORIGIN = 0x00000400, LENGTH = 128K - 0x400
    sram(rwx)      : ORIGIN = 0x1ffff000, LENGTH = 16K
}

_estack = ORIGIN(sram) + LENGTH(sram); /* 0x20003000, the end of sRAM */

SECTIONS {
    .vectors : { KEEP(*(.vectors)) } > interrupts /* vector table */
    .text : { *(.text*) } > flash /* firmware code */
    .rodata : { *(.rodata*) } > flash /* read-only data */

    /* use _sada and _edata in _reset() to copy data to sRAM */
    .data : {
        _sdata = .; /* start of .data section */
        *(.first_data)
        *(.data SORT(.data.*))
        _edata = .; /* end of .data section */
    } > sram AT > flash
    _sidata = LOADADDR(.data); /* load address of .data section */

    .bss : {
        _sbss = .; /* start of .bss section */
        *(.bss SORT(.bss.*) COMMON)
        _ebss = .; /* end of .bss section */
    } > sram

    . = ALIGN(8);
    _end = .;
}