/**
Start and end of flash and sram. The following is for
STM32f103c8t6 64KB variant with 20KB of SRAM
 */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    SRAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 20K
}

_estack = ORIGIN(SRAM) + LENGTH(SRAM); /*Stack end = SRAM_ORIGIN + SRAM_LENGTH*/

SECTIONS{
    .isr_vector :
    {
        KEEP(*(.isr_vector)) /*prevents */
    } > FLASH
    .text :
    {
        . = ALIGN(4);
        *(.text) /*Copy all text sections*/
    } > FLASH
    _sidata = LOADADDR(.data);
    .init_array :
    {
        . = ALIGN(4);
        _sinit = .;
        *(.init_array)
        _einit = .;
    } > FLASH
    .data :
    {
        . = ALIGN(4);
        _sdata = .;                /* The destination start address in SRAM */
        *(.data)    /*collect all input .data sections from all object files*/
        _edata = .;                /* The destination end address in SRAM */
    } > SRAM AT>FLASH /*Writes from load memory address of FLASH region into VMA of SRAM region*/

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        _ebss = .;
        end = .;
    } > SRAM
}