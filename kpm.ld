SECTIONS {
    . = 0;
    .kpm.info  : { *(.kpm.info) }
    .kpm.init  : { *(.kpm.init) }
    .kpm.exit  : { *(.kpm.exit) }
    .text      : { *(.text) }
    .rodata    : { *(.rodata) }
    .data      : { *(.data) }
    .bss       : { *(COMMON) *(.bss) }
}
