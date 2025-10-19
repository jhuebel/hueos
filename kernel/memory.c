#include "kernel.h"

static uint32_t memory_end;
static uint32_t placement_address = 0x100000; // Start at 1MB

void init_memory(struct multiboot_info* mbi) {
    terminal_writestring("Initializing memory management...\n");
    
    // Check if memory map is available
    if (mbi->flags & 0x40) {
        terminal_writestring("Memory map available\n");
        
        struct multiboot_mmap_entry* mmap = (struct multiboot_mmap_entry*) mbi->mmap_addr;
        
        while ((uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length) {
            if (mmap->type == 1) { // Available memory
                terminal_writestring("Available memory region found\n");
                if (mmap->addr + mmap->len > memory_end) {
                    memory_end = (uint32_t)(mmap->addr + mmap->len);
                }
            }
            mmap = (struct multiboot_mmap_entry*)((uint32_t)mmap + mmap->size + sizeof(uint32_t));
        }
    } else {
        // Fallback to basic memory info
        memory_end = (mbi->mem_upper + 1024) * 1024; // Convert KB to bytes
    }
    
    terminal_writestring("Memory initialization complete\n");
}

// Simple page frame allocator
uint32_t allocate_frame(void) {
    uint32_t frame = placement_address;
    placement_address += PAGE_SIZE;
    
    if (placement_address >= memory_end) {
        terminal_writestring("ERROR: Out of memory!\n");
        return 0;
    }
    
    return frame;
}

// Simple kernel heap allocator (very basic)
void* kmalloc(size_t size) {
    if (size == 0) return 0;
    
    // Align to 4-byte boundary
    size = (size + 3) & ~3;
    
    void* ret = (void*)placement_address;
    placement_address += size;
    
    if (placement_address >= memory_end) {
        terminal_writestring("ERROR: Kernel heap exhausted!\n");
        return 0;
    }
    
    return ret;
}