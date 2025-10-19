#ifndef HWINFO_H
#define HWINFO_H

#include "kernel.h"

// CPU Information structure
typedef struct {
    uint32_t vendor_id[4];
    char vendor_string[13];
    uint32_t family;
    uint32_t model;
    uint32_t stepping;
    uint32_t features_edx;
    uint32_t features_ecx;
} cpu_info_t;

// Function prototypes
void init_hwinfo(void);
void print_detailed_hardware_info(void);
void detect_cpu(void);
void print_cpu_info(void);
void print_memory_map(struct multiboot_info* mbi);
void scan_pci_bus(void);

#endif