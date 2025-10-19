#ifndef KERNEL_H
#define KERNEL_H

// Basic type definitions
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned long long uint64_t;

typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed int     int32_t;
typedef signed long long int64_t;

typedef unsigned long  size_t;
typedef unsigned long  uintptr_t;
#define NULL ((void*)0)

// Multiboot information structure
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    uint8_t  color_info[6];
} __attribute__((packed));

// Memory map entry
struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed));

// Function prototypes
void kernel_main(uint32_t magic, struct multiboot_info* mbi);
void init_gdt(void);
void init_idt(void);
void init_memory(struct multiboot_info* mbi);
void init_hyperv(void);
void init_serial(void);
void init_ide(void);
void init_hwinfo(void);
void init_vesa(const char* cmdline);
void init_vesa_with_mbi(const char* cmdline, struct multiboot_info* mbi);
void print_available_modes(void);
void framebuffer_init(struct multiboot_info* mbi);
int framebuffer_is_active(void);
void framebuffer_get_dimensions(size_t* width, size_t* height);
void framebuffer_putchar(char c);
void framebuffer_writestring(const char* str);
void framebuffer_set_color(uint8_t color);
void framebuffer_clear(void);
uint8_t reverse_bits(uint8_t b);
void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_set_dimensions(size_t width, size_t height);
void terminal_get_dimensions(size_t* width, size_t* height);
void serial_write(const char* data);
void serial_writechar(char c);
void ide_detect_devices(void);
void ide_print_devices(void);
void print_detailed_hardware_info(void);
void print_memory_map(struct multiboot_info* mbi);

// Global verbose mode flag
extern int kernel_verbose_mode;

// Port I/O functions
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
    asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Memory management
#define PAGE_SIZE 4096
#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_PAGE_NUMBER (KERNEL_VIRTUAL_BASE >> 22)

// Hyper-V specific definitions
#define HYPERV_CPUID_VENDOR_AND_MAX_FUNCTIONS   0x40000000
#define HYPERV_CPUID_INTERFACE                  0x40000001
#define HYPERV_CPUID_VERSION                    0x40000002
#define HYPERV_CPUID_FEATURES                   0x40000003
#define HYPERV_CPUID_ENLIGHTENMENT_INFO         0x40000004
#define HYPERV_CPUID_IMPLEMENT_LIMITS           0x40000005

#define HYPERV_HYPERVISOR_PRESENT_BIT           0x80000000
#define HYPERV_CPUID_MIN                        0x40000005
#define HYPERV_CPUID_MAX                        0x4000ffff

// Hyper-V hypercall interface
#define HV_X64_MSR_GUEST_OS_ID                  0x40000000
#define HV_X64_MSR_HYPERCALL                    0x40000001

#endif