# HueOS Video Mode Support

## Overview

HueOS includes support for multiple text display modes, including high-resolution VGA modes and wide-screen modes using graphics framebuffer rendering.

## Supported Video Modes

### 1. Standard VGA Text Mode (80x25)
- **Default mode** - Works everywhere including QEMU
- Uses standard VGA text mode at 0xB8000
- 16 colors, hardware text rendering
- **Status**: ✅ Fully working

### 2. High Resolution VGA Text Mode (80x50)
- Doubles vertical resolution using 8x8 font
- Uses VGA register programming to modify text mode
- Requires BIOS/bootloader support for mode switching
- **Status**: ✅ Code complete, requires real hardware for testing
- **QEMU Limitation**: GRUB command line parameters not passed in QEMU

### 3. Wide Screen Modes (132 columns) - Graphics Framebuffer
- **132x25** - Wide screen standard height (1056x400 pixels)
- **132x50** - Wide screen high resolution (1056x800 pixels)
- Uses graphics framebuffer with software text rendering
- Renders text pixel-by-pixel using 8x8 bitmap font
- **Status**: ✅ Code complete, requires real hardware with VESA support
- **QEMU Limitation**: Graphics mode not activated by GRUB in QEMU

## Implementation Details

### VGA 80x50 Mode
Located in `kernel/vesa.c`:
- Programs VGA CRTC registers for 400-line mode
- Loads 8x8 font into VGA plane 2
- Uses bit-reversed font data for correct display
- Function: `set_80x50_mode()`

### Graphics Framebuffer Renderer
Located in `kernel/framebuffer.c`:
- Detects graphics framebuffer from multiboot info
- Calculates text dimensions from pixel resolution
- Renders characters using 8x8 bitmap font
- Supports RGB pixel formats
- Auto-activates when multiboot provides graphics framebuffer

### Font Support
- 8x8 bitmap font in `kernel/font8x8.h`
- 128 ASCII characters (0x00-0x7F)
- Bit-reversed for correct pixel order
- Used by both VGA 80x50 mode and framebuffer renderer

## GRUB Boot Menu

The ISO includes multiple boot menu entries:

1. **HueOS** - Standard 80x25 VGA text mode (default)
2. **HueOS (Verbose)** - Detailed hardware info in 80x25 mode
3. **HueOS (80x50 High Resolution)** - High-res VGA text mode
4. **HueOS (132x25 Wide Screen)** - Wide screen with graphics framebuffer
5. **HueOS (132x50 Extra Lines)** - Wide screen high-res with graphics framebuffer

## Known Limitations

### QEMU/Emulator Limitations
1. **Command Line Parameters**: GRUB in QEMU doesn't pass multiboot command line parameters correctly (cmdline is always empty)
2. **Graphics Modes**: GRUB's `gfxpayload` settings don't activate graphics modes in QEMU's BIOS
3. **Testing**: High-resolution and wide-screen modes require testing on real hardware or better emulation

### Hardware Requirements
- **80x50 mode**: Any VGA-compatible hardware (post-1987)
- **132x modes**: VESA-compatible graphics card with appropriate resolution support
- **Recommended**: Modern PC with UEFI and VESA 2.0+ support

## Testing on Real Hardware

To test on real hardware:
1. Burn `hueos.iso` to USB drive: `dd if=hueos.iso of=/dev/sdX bs=4M`
2. Boot from USB drive
3. Select desired video mode from GRUB menu
4. Check serial output (COM1, 115200 baud) for diagnostics

## Architecture

```
Multiboot Info → init_vesa_with_mbi()
                      ↓
        ┌─────────────┴─────────────┐
        ↓                           ↓
   Text Mode (type 2)          Graphics Mode (type 1)
        ↓                           ↓
   VGA Driver                  Framebuffer Init
   (terminal.c)                (framebuffer.c)
        ↓                           ↓
   Hardware Text               Software Text Rendering
   @ 0xB8000                   Pixel-by-pixel
```

## Future Enhancements

- [ ] Multiboot2 support for better bootloader compatibility
- [ ] Additional font sizes (16x16 for higher DPI)
- [ ] More color depths (24-bit/32-bit)
- [ ] Mouse cursor support in graphics modes
- [ ] Scrolling and paging support
- [ ] Alternative bootloader (avoid GRUB limitations)

## References

- Multiboot Specification 0.6.96
- VGA Programming Documentation
- VESA BIOS Extensions (VBE) 3.0
- OSDev Wiki: Text Mode, VGA Hardware, Framebuffer
