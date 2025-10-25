#include "kernel.h"

int kernel_verbose_mode = 0;

// Simple string comparison
static int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Simple string search
static int strstr_check(const char* haystack, const char* needle) {
    if (!haystack || !needle) return 0;
    
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        
        if (!*n) return 1; // Found
        haystack++;
    }
    return 0; // Not found
}

void kernel_main(uint32_t magic, struct multiboot_info* mbi) {
    // Initialize serial port first for debugging
    init_serial();
    serial_write("Serial port initialized\n");
    
    // Initialize framebuffer if available
    framebuffer_init(mbi);
    
    // Initialize terminal for VGA output
    terminal_initialize();
    serial_write("Terminal initialized\n");
    
    // Check for resolution mode early (before printing)
    if (mbi->flags & 0x04) {
        char* cmdline = (char*)mbi->cmdline;
        init_vesa_with_mbi(cmdline, mbi);
    } else {
        init_vesa_with_mbi(NULL, mbi);
    }
    
    terminal_writestring("HueOS Kernel Starting...\n");
    serial_write("HueOS Kernel Starting...\n");
    terminal_writestring("========================\n");
    
    // Check multiboot magic number
    serial_write("Checking multiboot magic...\n");
    if (magic != 0x2BADB002) {
        terminal_writestring("ERROR: Invalid multiboot magic number!\n");
        serial_write("ERROR: Invalid multiboot magic number!\n");
        return;
    }
    
    terminal_writestring("Multiboot loader detected\n");
    terminal_writestring("Multiboot loader detected\n");
    serial_write("Multiboot loader detected\n");
    
    // Check for verbose mode from command line
    if (mbi->flags & 0x04) {
        char* cmdline = (char*)mbi->cmdline;
        if (cmdline && strstr_check(cmdline, "verbose")) {
            kernel_verbose_mode = 1;
            terminal_writestring("Verbose mode enabled\n");
            serial_write("Verbose mode enabled\n");
        }
    }
    
    // Initialize Global Descriptor Table
    terminal_writestring("Initializing GDT...\n");
    serial_write("Initializing GDT...\n");
    init_gdt();
    terminal_writestring("GDT initialized\n");
    serial_write("GDT initialized\n");
    
    // Initialize Interrupt Descriptor Table
    terminal_writestring("Initializing IDT...\n");
    serial_write("Initializing IDT...\n");
    init_idt();
    terminal_writestring("IDT initialized\n");
    serial_write("IDT initialized\n");
    
    // Initialize memory management
    serial_write("Initializing memory...\n");
    init_memory(mbi);
    serial_write("Memory initialized\n");
    
    // Initialize Hyper-V support
    serial_write("Initializing Hyper-V...\n");
    init_hyperv();
    serial_write("Hyper-V initialization complete\n");
    
    // Initialize hardware info
    serial_write("Detecting hardware...\n");
    init_hwinfo();
    
    // Initialize IDE controller
    serial_write("Initializing IDE...\n");
    init_ide();
    ide_detect_devices();
    
    // Initialize SCSI controller
    serial_write("Initializing SCSI...\n");
    init_scsi();
    scsi_scan_devices();
    
    // Print system information
    terminal_writestring("\nSystem Information:\n");
    terminal_writestring("==================\n");
    
    if (mbi->flags & 0x01) {
        terminal_writestring("Memory info available\n");
    }
    
    if (mbi->flags & 0x02) {
        terminal_writestring("Boot device info available\n");
    }
    
    if (mbi->flags & 0x04) {
        terminal_writestring("Command line available\n");
    }
    
    if (mbi->flags & 0x08) {
        terminal_writestring("Module info available\n");
    }
    
    // Display IDE devices
    ide_print_devices();
    
    // Display SCSI devices
    scsi_print_devices();
    
    // If verbose mode, show detailed hardware info
    if (kernel_verbose_mode) {
        print_detailed_hardware_info();
        print_memory_map(mbi);
    }
    
    terminal_writestring("\nHueOS kernel initialization complete!\n");
    terminal_writestring("Kernel is now running...\n");
    serial_write("\nHueOS kernel initialization complete!\n");
    serial_write("Kernel is now running...\n");
    
    // Main kernel loop
    while (1) {
        // In a real OS, this would be the scheduler
        asm volatile ("hlt"); // Halt until next interrupt
    }
}