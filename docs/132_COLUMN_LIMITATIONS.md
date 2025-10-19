# 132-Column Mode Limitations

## Current Status

The 132-column text mode support in HueOS faces platform-specific limitations due to the way VESA BIOS and bootloaders interact with different virtualization platforms and hardware.

## Technical Background

### Standard VGA Limitations
- Standard VGA hardware only supports 80-column text mode natively
- 132-column mode requires SVGA/VESA extensions
- True 132-column text modes (VESA modes 0x109, 0x10A, 0x10B) must be set via VESA BIOS

### The GRUB2 Challenge

GRUB2's video mode support has several limitations:

1. **Platform Dependency**: GRUB's ability to set VESA modes depends on:
   - BIOS implementation (real hardware vs virtualized)
   - Video BIOS extensions available
   - Bootloader video module support

2. **QEMU/KVM Limitations**:
   - QEMU's emulated BIOS has limited VESA mode support
   - Standard Bochs BIOS doesn't expose 132-column text modes
   - Graphics modes are available but text modes are restricted

3. **gfxpayload Behavior**:
   - `set gfxpayload=WxHxD` requests a specific mode
   - If mode unavailable, GRUB falls back to default (80x25)
   - No error reporting to the kernel about mode failures

## What Works

### Software Dimensions (All Platforms)
- Kernel correctly detects whatever mode GRUB sets
- Terminal driver can handle any column width in software
- Scrolling and text wrapping work correctly
- Command line parsing (`res=132x50`) functions properly

### VirtualBox/VMware
- Better VESA BIOS emulation
- More likely to support 132-column modes
- May require guest additions/tools

### Real Hardware
- Modern PC with VESA-compliant BIOS
- Should support 132-column text modes
- SVGA graphics cards typically support these modes

## Current Implementation

### What We've Implemented

1. **Multiboot Video Mode Detection**
   - Reads framebuffer info from GRUB
   - Supports both text (type 2) and graphics (type 1) modes
   - Calculates text dimensions from pixel resolution

2. **Dynamic Terminal Configuration**
   - Terminal driver adapts to detected dimensions
   - Supports arbitrary column widths
   - Proper cursor positioning and scrolling

3. **GRUB Menu Entries**
   - Attempts to set VESA graphics modes
   - Includes video driver modules (vbe, bochs, cirrus)
   - Falls back gracefully when modes unavailable

4. **Debug Output**
   - Serial console shows detected dimensions
   - Reports multiboot flags and framebuffer type
   - Helps diagnose mode switching issues

### What Doesn't Work (Yet)

1. **QEMU Default Configuration**
   - Cannot set 132-column modes
   - BIOS doesn't expose required VESA modes
   - Always falls back to 80x25

2. **Hardware Mode Switching from Protected Mode**
   - Can't call VESA BIOS INT 10h from 32-bit protected mode
   - Would require V8086 mode or real mode transition
   - Not implemented (complex and rarely needed)

## Workarounds and Solutions

###  Option 1: Use Alternative Virtualizer
```bash
# VirtualBox often has better VESA support
VBoxManage modifyvm "YourVM" --graphicscontroller vmsvga
```

### Option 2: QEMU with SeaBIOS Options
```bash
qemu-system-i386 -cdrom hueos.iso \\
    -vga std \\
    -display gtk,zoom-to-fit=on
```

### Option 3: Test on Real Hardware
- Boot from USB stick or CD
- Modern PCs should support VESA modes
- Verify BIOS has VESA extensions enabled

### Option 4: Custom GRUB Configuration
Edit `grub.cfg` to try different modes:
```
set gfxmode=1024x768x16  # Try different resolutions
set gfxpayload=keep
```

### Option 5: Framebuffer Graphics Mode (Future)
Implement a framebuffer terminal that:
- Uses graphics mode (reliably available)
- Renders text using software fonts
- Supports arbitrary column counts
- Would work on all platforms

## Future Enhancements

### Short Term
1. Add framebuffer graphics mode support
2. Implement software text rendering
3. Use multiboot framebuffer pointer directly

### Long Term
1. V8086 mode for VESA BIOS calls
2. Custom VGA programming for 132-column
3. GOP (UEFI Graphics Output Protocol) support

## Testing Results

### Platforms Tested

| Platform | 80x25 | 80x50 | 132x25 | 132x50 | Notes |
|----------|-------|-------|--------|--------|-------|
| QEMU/KVM | ✅ | ✅ | ❌ | ❌ | BIOS limitation |
| VirtualBox | ✅ | ✅ | ⚠️ | ⚠️ | Depends on settings |
| VMware | ✅ | ✅ | ⚠️ | ⚠️ | Requires VMware Tools |
| Real HW (UEFI) | ✅ | ✅ | ❓ | ❓ | Not tested |
| Real HW (BIOS) | ✅ | ✅ | ⚠️ | ⚠️ | Hardware dependent |

Legend:
- ✅ Works reliably
- ⚠️ May work with configuration
- ❌ Known not to work
- ❓ Not tested

## Recommendations

### For Development/Testing
- Use 80x25 or 80x50 modes (reliable everywhere)
- Test 132-column on VirtualBox or real hardware
- Serial console shows all debug info regardless of video mode

### For Production Use
- Provide multiple boot options (user can choose what works)
- Default to 80x25 (maximum compatibility)
- Document platform-specific requirements
- Consider implementing framebuffer text rendering

### For Contributors
- Focus on framebuffer graphics mode support
- Test on multiple platforms
- Document any new findings
- Share working configurations

## Conclusion

132-column text mode support is implemented in the kernel and works correctly when the bootloader can set the mode. The limitation is in GRUB's ability to request VESA modes on certain platforms (especially QEMU).

The software is ready - we just need better BIOS/bootloader mode setting, which is platform-dependent and outside the kernel's direct control.

**Bottom Line**: Works on some platforms, limited by BIOS/bootloader capabilities on others. Fallback to 80-column modes always available.
