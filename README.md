# HueOS - x86 Operating System with Hyper-V Support

[![Version](https://img.shields.io/badge/version-0.3.0--alpha-orange)](https://github.com/jhuebel/hueos/releases)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A minimal x86-based operating system kernel with UEFI/BIOS boot support and Hyper-V integration.

> **⚠️ Alpha Status**: HueOS is currently in active alpha development. Features are experimental and breaking changes may occur between releases. Not intended for production use.

## Current Version: v0.3.0-alpha

See [docs/VERSIONING.md](docs/VERSIONING.md) for versioning scheme and release history.

## Features

- **Multiboot compliance**: Compatible with GRUB2 bootloader
- **Hybrid boot support**: Boots on both BIOS (legacy) and UEFI (x86_64 + ia32) systems
- **Hyper-V compatible**: Supports both Generation 1 (BIOS) and Generation 2 (UEFI) VMs
- **SCSI storage**: LSI Logic 53C895A and BusLogic controller support
- **IDE/ATAPI driver**: Support for hard disks and optical drives
- **Multiple display modes**: Text resolutions from 80x25 to 132x50
- **Memory management**: Basic paging and heap allocation
- **Hardware abstraction**: GDT/IDT setup and interrupt handling
- **Hardware detection**: CPU info, PCI scanning, memory mapping
- **Serial port**: COM1 debugging support
- **Verbose boot mode**: Detailed hardware information display
- **Professional build system**: Separate build/, dist/ directories, package targets

## Architecture

### Boot Process
1. BIOS/UEFI loads GRUB2 bootloader
2. GRUB2 loads the kernel using multiboot protocol
3. Assembly bootstrap (`boot/multiboot.asm`) sets up initial environment
4. Kernel main function initializes core systems
5. Hyper-V detection and integration services activation

### Core Components

- **Boot loader**: Multiboot-compliant assembly bootstrap
- **Kernel**: C-based kernel with hardware abstraction
- **Memory management**: Simple page frame allocator and heap
- **Hyper-V support**: Hypervisor detection and hypercall interface
- **Terminal**: VGA text mode output with scrolling
- **Serial port**: COM1 serial debugging interface
- **IDE driver**: ATA/ATAPI device support with PIO mode
- **SCSI driver**: SCSI controller and device detection
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

# Build the kernel binary
make

# Create bootable hybrid BIOS+UEFI ISO
make iso

# Create distribution package (removes .o files for clean release)
make package

# Create distribution package (preserves .o files for faster rebuilds)
make preserve

# Run in QEMU (BIOS mode)
make qemu-iso

# Run in QEMU (UEFI mode)
make qemu-iso-uefi

# Debug with GDB
make debug

# Clean build artifacts
make clean
```

### Build Output

- **build/hueos.bin** - Kernel binary
- **build/hueos.iso** - Bootable ISO image (hybrid BIOS+UEFI)
- **build/*.o** - Object files (for incremental builds)
- **dist/hueos.iso** - Distribution ISO (created by `make package` or `make preserve`)
- **dist/hueos.bin** - Distribution kernel binary

## Project Structure

```
HueOS/
├── boot/
│   └── multiboot.asm      # Multiboot header and bootstrap
├── docs/                  # Documentation
│   ├── README.md          # Documentation index
│   ├── VERSIONING.md      # Version scheme and release process
│   ├── SCSI_SUPPORT.md    # SCSI storage documentation
│   ├── UEFI_SUPPORT.md    # UEFI boot documentation
│   ├── HYPERV_BOOT.md     # Hyper-V configuration guide
│   ├── QUICK_START_HYPERV.md  # Fast-track Hyper-V setup
│   ├── DISPLAY_MODES.md   # Display resolution options
│   ├── IDE_AND_HARDWARE.md    # IDE driver documentation
│   ├── TESTING.md         # Testing guide
│   ├── BOOT_ISSUE_RESOLUTION.md
│   ├── UPDATE_SUMMARY.md
│   └── archive/           # Historical documentation
├── include/
│   ├── kernel.h           # Kernel headers and definitions
│   ├── serial.h           # Serial port interface
│   ├── ide.h              # IDE/ATAPI driver interface
│   ├── scsi.h             # SCSI driver interface
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
│   ├── scsi.c            # SCSI storage driver
│   └── hwinfo.c          # Hardware detection
├── build/                # Build artifacts (gitignored)
│   ├── *.o               # Object files
│   ├── hueos.bin         # Kernel binary
│   └── hueos.iso         # Bootable ISO
├── dist/                 # Distribution packages (gitignored)
│   ├── hueos.bin         # Release kernel binary
│   └── hueos.iso         # Release ISO
├── linker.ld             # Linker script
├── Makefile              # Build system
├── VERSION               # Current version number
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
# BIOS mode with ISO
make qemu-iso

# UEFI mode with ISO (requires OVMF firmware)
make qemu-iso-uefi

# Direct kernel boot (no ISO needed)
make qemu
```

### Running on Hyper-V

HueOS supports both **Generation 1 (BIOS)** and **Generation 2 (UEFI)** Virtual Machines:

**Generation 1 VM (BIOS boot with IDE storage):**
1. Create a new Generation 1 VM in Hyper-V Manager
2. Attach `dist/hueos.iso` or `build/hueos.iso` to the DVD drive
3. Boot the VM

**Generation 2 VM (UEFI boot with SCSI storage):**
1. Create a new Generation 2 VM in Hyper-V Manager
2. Disable Secure Boot (in VM Settings → Security)
3. Attach `dist/hueos.iso` or `build/hueos.iso` to the DVD drive
4. Boot the VM

See [docs/QUICK_START_HYPERV.md](docs/QUICK_START_HYPERV.md) for fast-track setup or [docs/HYPERV_BOOT.md](docs/HYPERV_BOOT.md) for complete configuration details.

## Development

### Adding new features

1. Add source files to `kernel/` directory
2. Add header files to `include/` directory
3. Update `Makefile` if needed (usually automatic via wildcards)
4. Include headers in `include/kernel.h`
5. Initialize new subsystems in `kernel/main.c`
6. Test with `make qemu-iso` or `make qemu-iso-uefi`

### Debugging

Use the debug target to run with GDB:

```bash
make debug
```

This will start QEMU with GDB stub and connect GDB to debug the kernel.

Serial output is available on COM1 for debugging. Use `-serial stdio` in QEMU or view Hyper-V serial logs.

### Creating a Release

See [docs/VERSIONING.md](docs/VERSIONING.md) for the complete release process. Quick summary:

1. Update `VERSION` file
2. Update `docs/VERSIONING.md` with release notes
3. Commit changes: `git commit -m "Bump version to v0.X.Y-alpha"`
4. Create tag: `git tag -a v0.X.Y-alpha -m "Release notes..."`
5. Push: `git push origin main && git push origin v0.X.Y-alpha`
6. Package: `make package`
7. Create GitHub Release and attach `dist/hueos.iso`

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
- **[docs/VERSIONING.md](docs/VERSIONING.md)** - Version scheme and release process
- **[docs/QUICK_START_HYPERV.md](docs/QUICK_START_HYPERV.md)** - Fast-track Hyper-V setup guide
- **[docs/HYPERV_BOOT.md](docs/HYPERV_BOOT.md)** - Complete Hyper-V configuration guide
- **[docs/UEFI_SUPPORT.md](docs/UEFI_SUPPORT.md)** - UEFI boot support documentation
- **[docs/SCSI_SUPPORT.md](docs/SCSI_SUPPORT.md)** - SCSI storage controller support
- **[docs/DISPLAY_MODES.md](docs/DISPLAY_MODES.md)** - Display resolution options and configuration
- **[docs/IDE_AND_HARDWARE.md](docs/IDE_AND_HARDWARE.md)** - IDE driver and hardware detection docs
- **[docs/TESTING.md](docs/TESTING.md)** - Testing guide and expected behavior
- **[docs/BOOT_ISSUE_RESOLUTION.md](docs/BOOT_ISSUE_RESOLUTION.md)** - Boot troubleshooting
- **[docs/UPDATE_SUMMARY.md](docs/UPDATE_SUMMARY.md)** - Latest features and updates

## Release History

### v0.3.0-alpha (Current - October 2025)
- Clean project structure with organized documentation
- ISO build output moved to `build/` directory
- Added `package` and `preserve` build targets
- Professional OSS project layout
- Versioning scheme established

### v0.2.0-alpha (October 2025)
- SCSI storage driver (LSI Logic, BusLogic)
- PCI bus scanning for controllers
- Hyper-V Generation 2 VM support

### v0.1.0-alpha (October 2025)
- UEFI boot support via GRUB2
- Hybrid BIOS + UEFI bootable ISO
- x86_64-efi and i386-efi support

See [docs/VERSIONING.md](docs/VERSIONING.md) for complete version history.

## Contributing

HueOS is in active alpha development. Contributions are welcome! Areas for enhancement:

- **Interrupt handlers**: Full IDT implementation
- **Device drivers**: Network cards, USB, sound
- **File systems**: FAT32, ext2, or custom filesystem
- **Memory management**: Virtual memory, better allocators
- **Process management**: Multitasking, scheduling
- **Network stack**: TCP/IP implementation
- **Advanced Hyper-V features**: Synthetic devices, enhanced integration

Please open an issue before starting major work to discuss the approach.

## Acknowledgments

- GRUB2 bootloader for multiboot support
- QEMU for testing and development
- OSDev community for documentation and resources

## Links

- **GitHub Repository**: https://github.com/jhuebel/hueos
- **Releases**: https://github.com/jhuebel/hueos/releases
- **Issues**: https://github.com/jhuebel/hueos/issues