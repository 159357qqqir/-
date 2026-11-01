SECTIONS {
    . = 0;
    .kpm_info  : { *(.kpm_info) }
    .kpm_init  : { *(.kpm_init) }
    .kpm_exit  : { *(.kpm_exit) }
    .text      : { *(.text) }
    .rodata    : { *(.rodata) }
    .data      : { *(.data) }
    .bss       : { *(COMMON) *(.bss) }
}
