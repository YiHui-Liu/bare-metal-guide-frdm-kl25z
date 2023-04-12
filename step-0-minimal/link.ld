ENTRY(_reset);
MEMORY {
    /* flash(rx): ORIGIN = 0x000000c4, LENGTH = 128K */
    flash(rx): ORIGIN = 0x00000000, LENGTH = 128K
    sram(rx): ORIGIN = 0x20000000, LENGTH = 12K
}

_estack = ORIGIN(sram) + LENGTH(sram); /* 0x20003000, the end of sRAM */

SECTIONS {
    .vectors : { KEEP(*(.vectors)) } > flash /* vector table */
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