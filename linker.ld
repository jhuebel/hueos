/* Linker script for HueOS kernel */

ENTRY(_start)

SECTIONS
{
    /* Kernel starts at 1MB */
    . = 0x00100000;
    
    /* Multiboot header comes first */
    .multiboot :
    {
        *(.multiboot)
    }
    
    /* Code section */
    .text ALIGN(4K) :
    {
        *(.text)
    }
    
    /* Read-only data */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }
    
    /* Initialized data */
    .data ALIGN(4K) :
    {
        *(.data)
    }
    
    /* Uninitialized data */
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }
    
    /* End of kernel marker */
    kernel_end = .;
}