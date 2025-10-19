#include "hwinfo.h"
#include "kernel.h"

static cpu_info_t cpu_info;
static int verbose_mode = 0;

// Helper to execute CPUID instruction
static inline void cpuid_exec(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    asm volatile("cpuid"
                 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                 : "a"(leaf));
}

void init_hwinfo(void) {
    detect_cpu();
}

void detect_cpu(void) {
    uint32_t eax, ebx, ecx, edx;
    
    // Get vendor ID
    cpuid_exec(0, &eax, &ebx, &ecx, &edx);
    cpu_info.vendor_id[0] = ebx;
    cpu_info.vendor_id[1] = edx;
    cpu_info.vendor_id[2] = ecx;
    cpu_info.vendor_id[3] = 0;
    
    // Convert to string
    char* vendor = cpu_info.vendor_string;
    for (int i = 0; i < 4; i++) vendor[i] = (char)(ebx >> (i * 8));
    for (int i = 0; i < 4; i++) vendor[i + 4] = (char)(edx >> (i * 8));
    for (int i = 0; i < 4; i++) vendor[i + 8] = (char)(ecx >> (i * 8));
    vendor[12] = '\0';
    
    // Get features
    if (eax >= 1) {
        cpuid_exec(1, &eax, &ebx, &ecx, &edx);
        
        cpu_info.stepping = eax & 0xF;
        cpu_info.model = (eax >> 4) & 0xF;
        cpu_info.family = (eax >> 8) & 0xF;
        
        cpu_info.features_edx = edx;
        cpu_info.features_ecx = ecx;
    }
}

void print_cpu_info(void) {
    terminal_writestring("\nCPU Information:\n");
    terminal_writestring("================\n");
    serial_write("\nCPU Information:\n");
    
    terminal_writestring("Vendor: ");
    terminal_writestring(cpu_info.vendor_string);
    terminal_writestring("\n");
    
    serial_write("Vendor: ");
    serial_write(cpu_info.vendor_string);
    serial_write("\n");
    
    // Family/Model/Stepping
    terminal_writestring("Family: ");
    terminal_putchar('0' + ((cpu_info.family / 10) % 10));
    terminal_putchar('0' + (cpu_info.family % 10));
    terminal_writestring(" Model: ");
    terminal_putchar('0' + ((cpu_info.model / 10) % 10));
    terminal_putchar('0' + (cpu_info.model % 10));
    terminal_writestring(" Stepping: ");
    terminal_putchar('0' + (cpu_info.stepping % 10));
    terminal_writestring("\n");
    
    // Features
    terminal_writestring("Features: ");
    if (cpu_info.features_edx & (1 << 0)) terminal_writestring("FPU ");
    if (cpu_info.features_edx & (1 << 4)) terminal_writestring("TSC ");
    if (cpu_info.features_edx & (1 << 5)) terminal_writestring("MSR ");
    if (cpu_info.features_edx & (1 << 6)) terminal_writestring("PAE ");
    if (cpu_info.features_edx & (1 << 9)) terminal_writestring("APIC ");
    if (cpu_info.features_edx & (1 << 23)) terminal_writestring("MMX ");
    if (cpu_info.features_edx & (1 << 25)) terminal_writestring("SSE ");
    if (cpu_info.features_edx & (1 << 26)) terminal_writestring("SSE2 ");
    if (cpu_info.features_ecx & (1 << 0)) terminal_writestring("SSE3 ");
    if (cpu_info.features_ecx & (1 << 31)) terminal_writestring("HYPERVISOR ");
    terminal_writestring("\n");
}

void print_memory_map(struct multiboot_info* mbi) {
    terminal_writestring("\nMemory Map:\n");
    terminal_writestring("===========\n");
    serial_write("\nMemory Map:\n");
    
    if (!(mbi->flags & 0x01)) {
        terminal_writestring("Memory info not available\n");
        return;
    }
    
    terminal_writestring("Lower memory: ");
    uint32_t lower = mbi->mem_lower;
    char num_str[16];
    int pos = 0;
    
    if (lower == 0) {
        num_str[pos++] = '0';
    } else {
        char digits[16];
        int d = 0;
        while (lower > 0) {
            digits[d++] = '0' + (lower % 10);
            lower /= 10;
        }
        for (int j = d - 1; j >= 0; j--) {
            num_str[pos++] = digits[j];
        }
    }
    num_str[pos] = '\0';
    terminal_writestring(num_str);
    terminal_writestring(" KB\n");
    
    terminal_writestring("Upper memory: ");
    uint32_t upper = mbi->mem_upper;
    pos = 0;
    
    if (upper == 0) {
        num_str[pos++] = '0';
    } else {
        char digits[16];
        int d = 0;
        while (upper > 0) {
            digits[d++] = '0' + (upper % 10);
            upper /= 10;
        }
        for (int j = d - 1; j >= 0; j--) {
            num_str[pos++] = digits[j];
        }
    }
    num_str[pos] = '\0';
    terminal_writestring(num_str);
    terminal_writestring(" KB\n");
    
    // Total
    uint32_t total = (mbi->mem_lower + mbi->mem_upper) / 1024;
    terminal_writestring("Total RAM: ");
    pos = 0;
    
    if (total == 0) {
        num_str[pos++] = '0';
    } else {
        char digits[16];
        int d = 0;
        while (total > 0) {
            digits[d++] = '0' + (total % 10);
            total /= 10;
        }
        for (int j = d - 1; j >= 0; j--) {
            num_str[pos++] = digits[j];
        }
    }
    num_str[pos] = '\0';
    terminal_writestring(num_str);
    terminal_writestring(" MB\n");
}

// PCI configuration space access
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | 
                                   (func << 8) | (offset & 0xFC) | 
                                   0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void scan_pci_bus(void) {
    terminal_writestring("\nPCI Devices:\n");
    terminal_writestring("============\n");
    serial_write("\nPCI Devices:\n");
    
    int device_count = 0;
    
    for (uint8_t bus = 0; bus < 8; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t vendor_device = pci_read_config(bus, slot, func, 0);
                uint16_t vendor_id = vendor_device & 0xFFFF;
                uint16_t device_id = (vendor_device >> 16) & 0xFFFF;
                
                if (vendor_id == 0xFFFF || vendor_id == 0x0000)
                    continue;
                
                device_count++;
                
                // Get class code
                uint32_t class_rev = pci_read_config(bus, slot, func, 0x08);
                uint8_t class_code = (class_rev >> 24) & 0xFF;
                uint8_t subclass = (class_rev >> 16) & 0xFF;
                
                terminal_writestring("  ");
                terminal_putchar('0' + (bus / 10));
                terminal_putchar('0' + (bus % 10));
                terminal_writestring(":");
                terminal_putchar('0' + (slot / 10));
                terminal_putchar('0' + (slot % 10));
                terminal_writestring(".");
                terminal_putchar('0' + func);
                terminal_writestring(" - ");
                
                // Device description
                switch (class_code) {
                    case 0x00: terminal_writestring("Unclassified"); break;
                    case 0x01: terminal_writestring("Mass Storage"); break;
                    case 0x02: terminal_writestring("Network"); break;
                    case 0x03: terminal_writestring("Display"); break;
                    case 0x04: terminal_writestring("Multimedia"); break;
                    case 0x05: terminal_writestring("Memory"); break;
                    case 0x06: terminal_writestring("Bridge"); break;
                    case 0x07: terminal_writestring("Communication"); break;
                    case 0x08: terminal_writestring("System"); break;
                    case 0x09: terminal_writestring("Input"); break;
                    case 0x0C: terminal_writestring("Serial Bus"); break;
                    default: terminal_writestring("Other");
                }
                
                if (class_code == 0x01) {
                    switch (subclass) {
                        case 0x01: terminal_writestring(" (IDE)"); break;
                        case 0x05: terminal_writestring(" (ATA)"); break;
                        case 0x06: terminal_writestring(" (SATA)"); break;
                        case 0x07: terminal_writestring(" (SAS)"); break;
                        case 0x08: terminal_writestring(" (NVMe)"); break;
                    }
                }
                
                terminal_writestring("\n");
            }
        }
    }
    
    if (device_count == 0) {
        terminal_writestring("No PCI devices found\n");
    }
}

void print_detailed_hardware_info(void) {
    terminal_writestring("\n");
    terminal_writestring("========================================\n");
    terminal_writestring("  DETAILED HARDWARE INFORMATION\n");
    terminal_writestring("========================================\n");
    serial_write("\n========================================\n");
    serial_write("DETAILED HARDWARE INFORMATION\n");
    serial_write("========================================\n");
    
    print_cpu_info();
    scan_pci_bus();
}