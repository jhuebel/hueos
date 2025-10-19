#include "kernel.h"

// CPUID function
static inline void cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    asm volatile("cpuid"
                 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                 : "a"(leaf));
}

// Read MSR
static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

// Write MSR
static inline void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

static int hyperv_detected = 0;
static uint32_t hyperv_features = 0;

int detect_hyperv(void) {
    uint32_t eax, ebx, ecx, edx;
    
    // Check for hypervisor presence
    cpuid(1, &eax, &ebx, &ecx, &edx);
    if (!(ecx & HYPERV_HYPERVISOR_PRESENT_BIT)) {
        return 0; // No hypervisor present
    }
    
    // Check for Hyper-V specifically
    cpuid(HYPERV_CPUID_VENDOR_AND_MAX_FUNCTIONS, &eax, &ebx, &ecx, &edx);
    
    // Check vendor signature "Microsoft Hv"
    if (ebx == 0x7263694D && ecx == 0x666F736F && edx == 0x76482074) {
        terminal_writestring("Hyper-V hypervisor detected\n");
        
        // Get Hyper-V features
        cpuid(HYPERV_CPUID_FEATURES, &eax, &ebx, &ecx, &edx);
        hyperv_features = eax;
        
        return 1;
    }
    
    return 0;
}

void init_hyperv_hypercalls(void) {
    if (!hyperv_detected) return;
    
    terminal_writestring("Initializing Hyper-V hypercalls...\n");
    
    // Set guest OS ID
    uint64_t guest_os_id = 0x0001000000000000ULL; // Basic guest OS ID
    wrmsr(HV_X64_MSR_GUEST_OS_ID, guest_os_id);
    
    // Initialize hypercall page (simplified)
    // In a real implementation, you'd allocate a proper page and set up hypercalls
    terminal_writestring("Hyper-V hypercalls initialized\n");
}

void init_hyperv_services(void) {
    if (!hyperv_detected) return;
    
    terminal_writestring("Initializing Hyper-V integration services...\n");
    
    // Initialize basic services
    if (hyperv_features & 0x01) {
        terminal_writestring("VP Runtime available\n");
    }
    
    if (hyperv_features & 0x02) {
        terminal_writestring("Partition Reference Time available\n");
    }
    
    if (hyperv_features & 0x04) {
        terminal_writestring("Basic SynIC MSRs available\n");
    }
    
    if (hyperv_features & 0x08) {
        terminal_writestring("Synthetic Timer MSRs available\n");
    }
    
    terminal_writestring("Hyper-V integration services initialized\n");
}

void init_hyperv(void) {
    terminal_writestring("Checking for Hyper-V support...\n");
    
    hyperv_detected = detect_hyperv();
    
    if (hyperv_detected) {
        init_hyperv_hypercalls();
        init_hyperv_services();
        terminal_writestring("Hyper-V initialization complete\n");
    } else {
        terminal_writestring("Hyper-V not detected, running on bare metal or other hypervisor\n");
    }
}