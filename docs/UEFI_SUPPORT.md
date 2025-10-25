# UEFI Boot Support

## Overview

HueOS now supports both **BIOS (Legacy)** and **UEFI** boot modes through GRUB2's multiboot protocol. The same ISO image (`hueos.iso`) can boot on both legacy BIOS systems and modern UEFI systems.

## What is UEFI?

UEFI (Unified Extensible Firmware Interface) is the modern replacement for legacy BIOS:
- Used by all modern PCs (post-2010)
- Required for Hyper-V Generation 2 VMs
- Supports Secure Boot, GPT partitions, and graphics
- Provides a richer boot environment

## Implementation

### Hybrid ISO

The ISO is created using `grub-mkrescue` which automatically generates:
- **El Torito boot image** for BIOS/Legacy boot
- **EFI System Partition** for UEFI boot (x86_64 and ia32)
- GRUB bootloader for both modes

### Boot Process

**BIOS Mode:**
1. BIOS loads MBR boot code
2. GRUB2 multiboot loads kernel
3. Kernel receives multiboot info structure
4. Standard HueOS initialization

**UEFI Mode:**
1. UEFI firmware loads GRUB2 EFI application
2. GRUB2 multiboot loads kernel  
3. Kernel receives multiboot info structure
4. Standard HueOS initialization

The kernel code is identical - GRUB handles the differences between BIOS and UEFI environments.

## Testing

### QEMU - BIOS Mode
```bash
make qemu-iso
# or manually:
qemu-system-i386 -cdrom hueos.iso
```

### QEMU - UEFI Mode
```bash
make qemu-iso-uefi
# or manually:
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom hueos.iso -m 256M
```

### Hyper-V

**Generation 1 VM (BIOS):**
- Supports: BIOS/Legacy boot
- DVD boot: Supported ✓
- Tested: Working ✓

**Generation 2 VM (UEFI):**
- Supports: UEFI boot only
- DVD boot: Supported ✓
- Tested: Ready for testing ✓
- Secure Boot: Disable in VM settings (unsigned kernel)

### Real Hardware

The ISO works on both legacy BIOS systems and modern UEFI PCs:

1. **Burn to USB:**
   ```bash
   sudo dd if=hueos.iso of=/dev/sdX bs=4M status=progress
   sync
   ```

2. **Boot settings:**
   - BIOS: Enable "Legacy Boot" or "CSM"
   - UEFI: Disable Secure Boot (unsigned kernel)

3. **Boot from USB**

## Build Requirements

### Required Packages
```bash
# Core build tools (already installed)
sudo apt-get install -y build-essential nasm

# GRUB tools for BIOS boot
sudo apt-get install -y grub-pc-bin xorriso

# GRUB tools for UEFI boot (NEW)
sudo apt-get install -y grub-efi-amd64-bin grub-efi-ia32-bin mtools

# QEMU for testing
sudo apt-get install -y qemu-system-x86 ovmf
```

### Verification
Check that GRUB has all platforms installed:
```bash
ls /usr/lib/grub/
# Should show: i386-pc x86_64-efi i386-efi
```

## Architecture Support

| Architecture | BIOS Boot | UEFI Boot | Status |
|--------------|-----------|-----------|--------|
| x86 (32-bit) | ✅ i386-pc | ✅ i386-efi | Supported |
| x86_64 (64-bit) | ✅ via i386-pc | ✅ x86_64-efi | Supported |

**Note:** The kernel itself is still 32-bit (i386). UEFI boot support means GRUB can load on UEFI systems, but the kernel remains 32-bit code.

## Advantages of UEFI Support

1. **Modern Hardware Compatibility**
   - Works on new PCs that may not have CSM/Legacy boot
   - Hyper-V Generation 2 VM support
   - Better hardware support (GOP graphics, ACPI tables)

2. **Future-Proof**
   - Industry standard for new systems
   - Easier to add 64-bit kernel support later
   - Path to Secure Boot support

3. **Single ISO**
   - One image for both BIOS and UEFI
   - Simplified distribution
   - Easier testing

## Troubleshooting

### ISO doesn't boot in UEFI mode
- Ensure Secure Boot is disabled
- Check that `/EFI/BOOT/BOOTX64.EFI` exists in ISO
- Try ia32 UEFI if x86_64 fails

### "No suitable video mode found" error
- This is a warning, not a failure
- GRUB falls back to text mode
- Kernel boots successfully

### Hyper-V Generation 2 issues
- Disable Secure Boot in VM settings
- Ensure DVD is attached before boot
- Check boot order in firmware settings

## Current Limitations

1. **32-bit Kernel Only**
   - Kernel is i386, not x86_64
   - UEFI just handles boot, not kernel architecture
   - Future: 64-bit kernel port possible

2. **No Secure Boot**
   - Kernel is unsigned
   - Secure Boot must be disabled
   - Future: Can add signing process

3. **Graphics Mode**
   - UEFI GOP (Graphics Output Protocol) not yet used
   - Falls back to GRUB text mode
   - Future: Can utilize UEFI framebuffer

## Future Enhancements

- [ ] Use UEFI GOP for better graphics
- [ ] 64-bit kernel support
- [ ] Secure Boot signing
- [ ] ACPI table parsing from UEFI
- [ ] UEFI runtime services usage
- [ ] NVR AM variable support

## References

- UEFI Specification 2.10
- GRUB2 Multiboot Specification
- Hyper-V Generation 2 VM Documentation
- OSDev Wiki: UEFI

## See Also

- [VIDEO_MODES.md](VIDEO_MODES.md) - Display mode support
- [README.md](README.md) - General documentation
- [docs/HYPERV_BOOT.md](docs/HYPERV_BOOT.md) - Hyper-V specific instructions
