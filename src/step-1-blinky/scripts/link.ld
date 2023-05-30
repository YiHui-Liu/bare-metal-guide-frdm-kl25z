ENTRY(_reset);
MEMORY {
    interrupt(rx)  : ORIGIN = 0x00000000, LENGTH = 0xC0
    cfmprotrom(rx) : ORIGIN = 0x00000400, LENGTH = 0x10
    flash(rx)      : ORIGIN = 0x00000410, LENGTH = 128K - 0x410
    sram(rwx)      : ORIGIN = 0x1ffff000, LENGTH = 16K
}

_estack = ORIGIN(sram) + LENGTH(sram); /* 0x20003000, the end of sRAM */

SECTIONS {
    /* vector table */
    .vectors : { 
        . = ALIGN(4);
        KEEP(*(.vectors));
        . = ALIGN(4);
    } > interrupt
    /* Flash Configuration Field (FCF) */
    .cfmprotect :
    {
        . = ALIGN(4);
        KEEP(*(.cfmconfig))
        . = ALIGN(4);
    } > cfmprotrom

    .text : { *(.text*) } > flash /* firmware code */
    .rodata : { *(.rodata*) } > flash /* read-only data */

    /* use _sada and _edata in _reset() to copy data to sRAM */
    .data : {
        _sdata = .; /* start of .data section */
        *(.first_data)
        *(.data SORT(.data.*))
        _edata = .; /* end of .data section */
    } > sram AT > flash
    _sidata = LOADADDR(.data);
    
    .bss : {
        _sbss = .; /* start of .bss section */
        *(.bss SORT(.bss.*) COMMON)
        _ebss = .; /* end of .bss section */
    } > sram

    _end = .;
}
