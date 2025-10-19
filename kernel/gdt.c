#include "kernel.h"

// GDT entry structure
struct gdt_entry {
    uint16_t limit_low;   // Lower 16 bits of limit
    uint16_t base_low;    // Lower 16 bits of base
    uint8_t  base_middle; // Next 8 bits of base
    uint8_t  access;      // Access flags
    uint8_t  granularity; // Granularity and upper limit
    uint8_t  base_high;   // Last 8 bits of base
} __attribute__((packed));

// GDT pointer structure
struct gdt_ptr {
    uint16_t limit;       // Upper 16 bits of all GDT segments
    uint32_t base;        // Address of first GDT entry
} __attribute__((packed));

// IDT entry structure
struct idt_entry {
    uint16_t base_low;    // Lower 16 bits of handler address
    uint16_t sel;         // Kernel segment selector
    uint8_t  always0;     // Always 0
    uint8_t  flags;       // Flags
    uint16_t base_high;   // Upper 16 bits of handler address
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;       // Upper 16 bits of all IDT segments
    uint32_t base;        // Address of first IDT entry
} __attribute__((packed));

// Arrays for GDT and IDT
struct gdt_entry gdt_entries[5];
struct gdt_ptr   gdt_ptr;
struct idt_entry idt_entries[256];
struct idt_ptr   idt_ptr;

// Assembly functions
extern void gdt_flush(uint32_t);
extern void idt_flush(uint32_t);

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;
    
    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    
    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

void init_gdt(void) {
    gdt_ptr.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;
    
    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment
    
    gdt_flush((uint32_t)&gdt_ptr);
}

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags;
}

void init_idt(void) {
    idt_ptr.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;
    
    // Clear IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Set up exception handlers (basic)
    // In a full OS, you'd set up proper exception handlers here
    
    idt_flush((uint32_t)&idt_ptr);
}