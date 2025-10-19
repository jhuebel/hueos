# HueOS Documentation

Welcome to the HueOS documentation! This folder contains detailed guides and technical documentation for the HueOS operating system.

## Quick Links

### Getting Started
- **[../README.md](../README.md)** - Main project overview and quick start guide
- **[QUICK_START_HYPERV.md](QUICK_START_HYPERV.md)** - Step-by-step guide for running HueOS on Hyper-V

### Troubleshooting & Setup
- **[BOOT_ISSUE_RESOLUTION.md](BOOT_ISSUE_RESOLUTION.md)** - Detailed analysis of boot issues and solutions
- **[HYPERV_BOOT.md](HYPERV_BOOT.md)** - Comprehensive Hyper-V boot guide and troubleshooting
- **[TESTING.md](TESTING.md)** - Testing guide and expected behavior

### Feature Documentation
- **[IDE_AND_HARDWARE.md](IDE_AND_HARDWARE.md)** - IDE/ATAPI driver and hardware detection documentation
- **[DISPLAY_MODES.md](DISPLAY_MODES.md)** - High resolution text display modes and configuration
- **[UPDATE_SUMMARY.md](UPDATE_SUMMARY.md)** - Latest feature updates and changes

## Documentation Overview

### For New Users

1. Start with **[../README.md](../README.md)** for basic overview
2. Read **[QUICK_START_HYPERV.md](QUICK_START_HYPERV.md)** if using Hyper-V
3. Check **[TESTING.md](TESTING.md)** for what to expect when booting

### For Troubleshooting

1. Boot issues? → **[BOOT_ISSUE_RESOLUTION.md](BOOT_ISSUE_RESOLUTION.md)**
2. Hyper-V problems? → **[HYPERV_BOOT.md](HYPERV_BOOT.md)**
3. Want to understand the kernel? → **[TESTING.md](TESTING.md)**

### For Developers

1. Latest features → **[UPDATE_SUMMARY.md](UPDATE_SUMMARY.md)**
2. IDE driver API → **[IDE_AND_HARDWARE.md](IDE_AND_HARDWARE.md)**
3. Hardware detection → **[IDE_AND_HARDWARE.md](IDE_AND_HARDWARE.md)**
4. Display modes → **[DISPLAY_MODES.md](DISPLAY_MODES.md)**

## Document Descriptions

### BOOT_ISSUE_RESOLUTION.md
**Purpose**: Explains why HueOS won't boot on Hyper-V Generation 2 VMs  
**Key Topics**:
- Root cause analysis
- BIOS vs UEFI boot
- Generation 1 vs Generation 2 VMs
- Proof the kernel works
- Platform compatibility matrix

### HYPERV_BOOT.md
**Purpose**: Complete guide for booting HueOS on Hyper-V  
**Key Topics**:
- Hyper-V VM configuration
- Generation 1 vs Generation 2 differences
- Step-by-step VM creation
- PowerShell automation scripts
- Serial output capture

### IDE_AND_HARDWARE.md
**Purpose**: Documentation for IDE/ATAPI driver and hardware detection  
**Key Topics**:
- IDE driver capabilities
- ATA hard disk support
- ATAPI optical drive support
- Hardware detection features
- API documentation
- Usage examples
- Testing procedures

### QUICK_START_HYPERV.md
**Purpose**: Fast-track guide to get HueOS running on Hyper-V  
**Key Topics**:
- TL;DR solution
- PowerShell one-liners
- GUI step-by-step
- Expected output
- Common troubleshooting

### TESTING.md
**Purpose**: Testing guide and expected behavior  
**Key Topics**:
- Build and test procedures
- Expected boot output
- Hyper-V detection
- Testing on various platforms
- Success metrics

### UPDATE_SUMMARY.md
**Purpose**: Latest features and changes summary  
**Key Topics**:
- New IDE/ATAPI driver
- Verbose boot mode
- Hardware detection
- GRUB boot options
- Testing results
- Code statistics

### DISPLAY_MODES.md
**Purpose**: Display resolution options and configuration  
**Key Topics**:
- Available text resolutions (80x25 to 132x50)
- GRUB menu options for different modes
- Resolution auto-detection
- Platform compatibility
- API reference for display functions
- Troubleshooting display issues

## Quick Reference

### Boot Options

HueOS provides multiple boot modes via GRUB:

1. **Normal Boot** - Standard 80x25 text mode
2. **Verbose Boot** - Detailed hardware information
3. **High Resolution Modes** - 80x50, 132x25, 132x43, 132x50 text modes
4. **Combined Modes** - Verbose with high resolution

See [DISPLAY_MODES.md](DISPLAY_MODES.md) for all available options.

### Supported Platforms

| Platform | Status | Documentation |
|----------|--------|---------------|
| QEMU | ✅ Works | [TESTING.md](TESTING.md) |
| Hyper-V Gen 1 | ✅ Works | [HYPERV_BOOT.md](HYPERV_BOOT.md) |
| Hyper-V Gen 2 | ❌ UEFI Required | [BOOT_ISSUE_RESOLUTION.md](BOOT_ISSUE_RESOLUTION.md) |
| VirtualBox | ✅ Works | [TESTING.md](TESTING.md) |
| VMware | ✅ Works | [TESTING.md](TESTING.md) |

### Key Features

- ✅ Multiboot/GRUB2 compatible
- ✅ Hyper-V integration
- ✅ IDE/ATAPI driver
- ✅ Hardware detection
- ✅ Multiple text display resolutions (80x25 to 132x50)
- ✅ VGA text mode with auto-detection
- ✅ Serial port debugging

## Building and Testing

```bash
# Build kernel
make

# Create ISO
make iso

# Test in QEMU
make qemu

# Test with verbose mode
qemu-system-i386 -kernel build/hueos.bin -append "verbose"

# Run test suite
./test-features.sh
```

See [../README.md](../README.md) for complete build instructions.

## Contributing Documentation

When adding new documentation:

1. Place `.md` files in the `docs/` folder
2. Update this index file
3. Add cross-references in relevant documents
4. Follow existing formatting style
5. Include code examples where applicable

## Need Help?

1. Check the relevant documentation above
2. Review [BOOT_ISSUE_RESOLUTION.md](BOOT_ISSUE_RESOLUTION.md) for common issues
3. Examine serial output logs for debugging
4. Read source code comments in `kernel/` and `include/` folders

---

**Last Updated**: October 19, 2025  
**HueOS Version**: 0.1.0  
**Documentation Version**: 1.0