# HueOS - x86 Operating System with Hyper-V Support

A minimal x86-based operating system kernel with basic Hyper-V hypervisor integration.

## Features

- **Multiboot compliance**: Compatible with GRUB2 bootloader
- **Memory management**: Basic paging and heap allocation
- **Hardware abstraction**: GDT/IDT setup and interrupt handling
- **Hyper-V integration**: Detection and basic integration services
- **Multiple display modes**: Text resolutions from 80x25 to 132x50
- **VGA text mode**: Auto-detecting terminal output with scrolling
- **Serial port**: COM1 debugging support
- **IDE/ATAPI driver**: Support for hard disks and optical drives
- **Hardware detection**: CPU info, PCI scanning, memory mapping
- **Verbose boot mode**: Detailed hardware information display
- **Cross-platform build**: Makefile-based build system

## Architecture

### Boot Process
1. GRUB2 loads the kernel using multiboot protocol
2. Assembly bootstrap (`boot/multiboot.asm`) sets up initial environment
3. Kernel main function initializes core systems
4. Hyper-V detection and integration services activation

### Core Components

- **Boot loader**: Multiboot-compliant assembly bootstrap
- **Kernel**: C-based kernel with hardware abstraction
- **Memory management**: Simple page frame allocator and heap
- **Hyper-V support**: Hypervisor detection and hypercall interface
- **Terminal**: VGA text mode output with scrolling
- **Serial port**: COM1 serial debugging interface
- **IDE driver**: ATA/ATAPI device support with PIO mode
- **Hardware detection**: CPUID, PCI scanning, device enumeration

## Building

### Prerequisites

- GCC cross-compiler (i686-elf-gcc recommended)
- NASM assembler
- GNU Make
- QEMU (for testing)
- GRUB2 tools (for ISO creation)

### Installation on Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install build-essential nasm qemu-system-x86 grub-pc-bin grub-common xorriso
```

### Building the kernel

```bash
# Check if all tools are available
make check-tools

# Build the kernel
make

# Create bootable ISO
make iso

# Run in QEMU
make qemu

# Run with Hyper-V optimizations (if on Windows with Hyper-V)
make qemu-hyperv

# Debug with GDB
make debug
```

## File Structure

```
HueOS/
├── boot/
│   └── multiboot.asm      # Multiboot header and bootstrap
├── docs/                  # Documentation
│   ├── README.md          # Documentation index
│   ├── BOOT_ISSUE_RESOLUTION.md
│   ├── DISPLAY_MODES.md   # Display resolution options
│   ├── HYPERV_BOOT.md
│   ├── IDE_AND_HARDWARE.md
│   ├── QUICK_START_HYPERV.md
│   ├── TESTING.md
│   └── UPDATE_SUMMARY.md
├── include/
│   ├── kernel.h           # Kernel headers and definitions
│   ├── serial.h           # Serial port interface
│   ├── ide.h              # IDE/ATAPI driver interface
│   └── hwinfo.h           # Hardware detection interface
├── kernel/
│   ├── main.c            # Kernel entry point
│   ├── terminal.c        # VGA text mode terminal
│   ├── vesa.c            # VESA/display mode support
│   ├── gdt.c             # Global Descriptor Table
│   ├── gdt_asm.asm       # GDT assembly support
│   ├── memory.c          # Memory management
│   ├── hyperv.c          # Hyper-V integration
│   ├── serial.c          # Serial port driver
│   ├── ide.c             # IDE/ATAPI driver
│   └── hwinfo.c          # Hardware detection
├── linker.ld             # Linker script
├── Makefile              # Build system
└── README.md             # This file
```

## Hyper-V Integration

The kernel includes basic Hyper-V integration features:

- **Hypervisor detection**: Uses CPUID to detect Hyper-V presence
- **Hypercall interface**: Basic MSR setup for hypercalls
- **Integration services**: Detection of available Hyper-V features
- **VP Runtime**: Virtual processor runtime support
- **Synthetic timers**: Time management in virtualized environment

### Hyper-V Features Detected

- VP Runtime availability
- Partition Reference Time
- Basic SynIC MSRs
- Synthetic Timer MSRs

## Testing

### Running on QEMU

```bash
# Standard QEMU emulation
make qemu

# With KVM acceleration (Linux)
make qemu-hyperv
```

### Running on Hyper-V

**IMPORTANT**: HueOS requires a **Generation 1 Virtual Machine** in Hyper-V.

1. Create a new Generation 1 VM in Hyper-V Manager
2. Disable Secure Boot (in VM Settings → Security)
3. Attach `hueos.iso` to the DVD drive
4. Boot the VM

**Why Generation 1?** The current bootloader uses BIOS/Multiboot, which is only supported by Generation 1 VMs. Generation 2 VMs require UEFI boot (future enhancement).

See [docs/HYPERV_BOOT.md](docs/HYPERV_BOOT.md) for detailed instructions and troubleshooting.

### Running on QEMU

```bash
# Standard QEMU emulation
make qemu

# With KVM acceleration (Linux)
make qemu-hyperv
```

### Adding new features

1. Add source files to appropriate directories
2. Update Makefile if needed
3. Include headers in `include/kernel.h`
4. Initialize new subsystems in `kernel/main.c`

### Debugging

Use the debug target to run with GDB:

```bash
make debug
```

This will start QEMU with GDB stub and connect GDB to debug the kernel.

## Technical Details

### Memory Layout

- Kernel loads at 1MB (0x100000)
- Stack grows downward from initial setup
- Heap grows upward from kernel end
- VGA buffer at 0xB8000

### Interrupt Handling

Basic IDT setup is provided but no interrupt handlers are implemented yet. This is a minimal bootstrap that can be extended.

### Hyper-V Hypercalls

The kernel sets up the basic infrastructure for Hyper-V hypercalls but doesn't implement specific hypercall functionality yet.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Copyright (c) 2025 Jason Huebel

## Documentation

Complete documentation is available in the [`docs/`](docs/) folder:

- **[docs/README.md](docs/README.md)** - Documentation index and quick links
- **[docs/QUICK_START_HYPERV.md](docs/QUICK_START_HYPERV.md)** - Fast-track Hyper-V setup guide
- **[docs/HYPERV_BOOT.md](docs/HYPERV_BOOT.md)** - Complete Hyper-V configuration guide
- **[docs/DISPLAY_MODES.md](docs/DISPLAY_MODES.md)** - Display resolution options and configuration
- **[docs/IDE_AND_HARDWARE.md](docs/IDE_AND_HARDWARE.md)** - IDE driver and hardware detection docs
- **[docs/TESTING.md](docs/TESTING.md)** - Testing guide and expected behavior
- **[docs/BOOT_ISSUE_RESOLUTION.md](docs/BOOT_ISSUE_RESOLUTION.md)** - Boot troubleshooting
- **[docs/UPDATE_SUMMARY.md](docs/UPDATE_SUMMARY.md)** - Latest features and updates

## Contributing

This is a minimal OS bootstrap. Contributions for additional features like:

- Interrupt handlers
- Device drivers
- File systems
- Network stack
- Advanced Hyper-V features

are welcome!