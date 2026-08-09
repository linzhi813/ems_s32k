/**
 * @file    S32K344_flash.ld
 * @brief   Linker script for NXP S32K344 (Cortex-M7, 2MB P-Flash, 256KB SRAM)
 *
 * Memory layout:
 *   P-Flash: 0x00400000 - 0x005FFFFF (2MB, 8KB erase sectors)
 *   SRAM:    0x20400000 - 0x2043FFFF (256KB, ECC)
 *   ITCM:    0x00000000 - 0x0000FFFF (64KB)
 *   DTCM:    0x20000000 - 0x2000FFFF (64KB)
 */

MEMORY
{
    /* IVT header + application code */
    FLASH      (rx)  : ORIGIN = 0x00400000, LENGTH = 2048K

    /* Main system SRAM (256KB, ECC protected) */
    SRAM       (rwx) : ORIGIN = 0x20400000, LENGTH = 256K

    /* Instruction Tightly Coupled Memory (64KB) */
    ITCM       (rwx) : ORIGIN = 0x00000000, LENGTH = 64K

    /* Data Tightly Coupled Memory (64KB) */
    DTCM       (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

/* Stack and heap sizes */
__stack_size__  = DEFINED(__stack_size__)  ? __stack_size__  : 0x00000800;  /* 2KB default */
__heap_size__   = DEFINED(__heap_size__)   ? __heap_size__   : 0x00000400;  /* 1KB default */

ENTRY(Reset_Handler)

SECTIONS
{
    /**************************************************************************
     * IVT Header (Image Vector Table)
     *
     * S32K3 boot ROM looks for IVT at the beginning of flash.
     * Must be in its own section at the very start of the image.
     * Size: exactly 256 bytes (0x100)
     **************************************************************************/
    .ivt_header ORIGIN(FLASH) :
    {
        KEEP(*(.ivt_header))
        . = ALIGN(0x100);
    } > FLASH

    /**************************************************************************
     * ARM Vector Table
     *
     * Must be aligned to power-of-two boundary matching total vector count.
     * For up to 272 vectors (16 system + 256 IRQ), align to 0x400 (1KB).
     * Placed immediately after IVT at 0x00400100.
     **************************************************************************/
    .vectors :
    {
        . = ALIGN(0x400);
        __vector_start__ = .;
        KEEP(*(.vectors))
        __vector_end__ = .;
    } > FLASH

    /**************************************************************************
     * Application code
     **************************************************************************/
    .text :
    {
        . = ALIGN(4);
        __text_start__ = .;
        *(.text)
        *(.text.*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        __text_end__ = .;
    } > FLASH

    /**************************************************************************
     * Read-only data and constants
     **************************************************************************/
    .rodata :
    {
        . = ALIGN(4);
        __rodata_start__ = .;
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
        __rodata_end__ = .;
    } > FLASH

    /**************************************************************************
     * Pre-initialization array (called before main)
     **************************************************************************/
    .preinit_array :
    {
        . = ALIGN(4);
        __preinit_array_start__ = .;
        KEEP(*(.preinit_array*))
        __preinit_array_end__ = .;
    } > FLASH

    /**************************************************************************
     * Initialization array (called before main)
     **************************************************************************/
    .init_array :
    {
        . = ALIGN(4);
        __init_array_start__ = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end__ = .;
    } > FLASH

    /**************************************************************************
     * Finalization array (called after main exits)
     **************************************************************************/
    .fini_array :
    {
        . = ALIGN(4);
        __fini_array_start__ = .;
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        __fini_array_end__ = .;
    } > FLASH

    /**************************************************************************
     * ARM exception index table (for efficient exception handling)
     **************************************************************************/
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start__ = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end__ = .;
    } > FLASH

    /**************************************************************************
     * Copy table (startup code uses this to copy data from FLASH to SRAM)
     **************************************************************************/
    __etext = .;

    /**************************************************************************
     * Initialized data (in SRAM, loaded from FLASH at __etext)
     **************************************************************************/
    .data : AT(__etext)
    {
        . = ALIGN(4);
        __data_start__ = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        __data_end__ = .;
    } > SRAM

    /**************************************************************************
     * Uninitialized data (BSS) - zeroed by startup code
     **************************************************************************/
    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > SRAM

    /**************************************************************************
     * Heap section
     *
     * Symbols used by _sbrk() in syscalls.c for malloc() support.
     * Heap grows upward from __heap_start__ toward __heap_end__.
     * Stack resides above __heap_end__, in the remaining SRAM.
     **************************************************************************/
    .heap :
    {
        . = ALIGN(8);
        __heap_start__ = .;
        . = . + __heap_size__;
        . = ALIGN(8);
        __heap_end__ = .;
    } > SRAM

    /**************************************************************************
     * Stack section
     *
     * __stack_top__ is the initial stack pointer loaded by Reset_Handler.
     * Stack grows downward from __stack_top__ toward __heap_end__.
     * Placed at the very end of SRAM to maximize heap space.
     *
     * We use SRAM end directly here instead of a .stack section
     * to avoid issues with section ordering.
     **************************************************************************/
    __stack_top__ = ORIGIN(SRAM) + LENGTH(SRAM);
    __stack_bottom__ = __stack_top__ - __stack_size__;

    /* Verify that heap and stack don't overlap at link time */
    ASSERT(__heap_end__ <= __stack_bottom__, "Error: heap and stack overflow collision")

    /**************************************************************************
     * ITCM section (fast code execution)
     *
     * Place critical ISR/hot-path code here using:
     *   __attribute__((section(".itcm_code"))) void my_fast_func(void) { ... }
     **************************************************************************/
    .itcm_code :
    {
        . = ALIGN(4);
        __itcm_start__ = .;
        *(.itcm_code)
        *(.itcm_code.*)
        . = ALIGN(4);
        __itcm_end__ = .;
    } > ITCM AT > FLASH
    __itcm_load_start__ = LOADADDR(.itcm_code);
    __itcm_load_end__ = __itcm_load_start__ + SIZEOF(.itcm_code);

    /**************************************************************************
     * DTCM section (fast data access)
     *
     * Place critical data here using:
     *   __attribute__((section(".dtcm_data"))) int my_fast_var;
     **************************************************************************/
    .dtcm_data :
    {
        . = ALIGN(4);
        __dtcm_start__ = .;
        *(.dtcm_data)
        *(.dtcm_data.*)
        . = ALIGN(4);
        __dtcm_end__ = .;
    } > DTCM AT > FLASH
    __dtcm_load_start__ = LOADADDR(.dtcm_data);
    __dtcm_load_end__ = __dtcm_load_start__ + SIZEOF(.dtcm_data);

    /* Discard unwanted sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
    }
}

/* Exported symbols for debugging */
__flash_start__     = ORIGIN(FLASH);
__flash_size__      = LENGTH(FLASH);
__sram_start__      = ORIGIN(SRAM);
__sram_size__       = LENGTH(SRAM);
__itcm_start_addr__ = ORIGIN(ITCM);
__itcm_size__       = LENGTH(ITCM);
__dtcm_start_addr__ = ORIGIN(DTCM);
__dtcm_size__       = LENGTH(DTCM);
