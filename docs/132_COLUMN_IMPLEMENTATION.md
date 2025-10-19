# 132-Column Text Mode Implementation

## Overview

This document describes the implementation of 132-column text mode support in HueOS, including wide screen resolutions like 132x25, 132x43, and 132x50.

## Challenge

The primary challenge with 132-column modes is that they are not standard VGA modes. Standard VGA hardware only natively supports:
- 80x25 (Mode 0x03)
- 80x50 (400-line mode with 8-pixel font)

132-column modes require **VESA BIOS Extensions (VBE)**, which provide access to additional video modes including:
- Mode 0x109: 132x25 (1056x400 pixels, 16 colors)
- Mode 0x10A: 132x43 (1056x688 pixels, 16 colors)
- Mode 0x10B: 132x50 (1056x800 pixels, 16 colors)
- Mode 0x10C: 132x60 (1056x960 pixels, 16 colors)

## Problem: Protected Mode Limitation

HueOS runs in **32-bit protected mode**, but VESA BIOS functions (INT 10h) only work in **real mode**. Once the kernel boots and switches to protected mode, direct BIOS calls are no longer possible.

## Solution: Multiboot Video Mode Specification

Rather than implementing complex V8086 mode or real mode transitions, we use **GRUB's built-in VESA support**:

1. **Multiboot Header Enhancement**: Updated `boot/multiboot.asm` to include video mode fields (flag bit 2)
2. **GRUB Configuration**: GRUB sets the desired VESA mode *before* loading the kernel
3. **Multiboot Info**: Kernel receives framebuffer information from GRUB via the multiboot info structure
4. **Auto-Detection**: Kernel reads the framebuffer dimensions and configures itself accordingly

## Implementation Details

### 1. Multiboot Header (boot/multiboot.asm)

```nasm
MULTIBOOT_FLAGS equ 0x00000007  ; Bits 0, 1, 2: page align + memory + video

; Video mode fields
dd 1    ; mode_type (1 = text mode, 0 = graphics)
dd 0    ; width (0 = no preference, GRUB decides)
dd 0    ; height (0 = no preference)
dd 0    ; depth (0 = no preference)
```

### 2. Expanded Multiboot Info Structure (include/kernel.h)

Added framebuffer fields to receive video mode information from GRUB:

```c
struct multiboot_info {
    // ... existing fields ...
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;    // Columns (e.g., 132)
    uint32_t framebuffer_height;   // Rows (e.g., 50)
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;     // 2 = text mode
    // ...
};
```

### 3. VESA Initialization (kernel/vesa.c)

New function `init_vesa_with_mbi()` checks multiboot framebuffer info:

```c
void init_vesa_with_mbi(const char* cmdline, struct multiboot_info* mbi) {
    // Check flag bit 12 for framebuffer info
    if (mbi && (mbi->flags & (1 << 12))) {
        // framebuffer_type == 2 means text mode
        if (mbi->framebuffer_type == 2) {
            size_t width = mbi->framebuffer_width;
            size_t height = mbi->framebuffer_height;
            terminal_set_dimensions(width, height);
            // Auto-load 8x8 font for 80x50 mode
            if (width == 80 && height == 50) {
                load_8x8_font();
            }
        }
    }
}
```

### 4. GRUB Configuration (Makefile)

GRUB menu entries use `set gfxmode` to request specific VESA modes:

```
menuentry "HueOS (132x25 Wide Screen)" {
    set gfxpayload=text
    set gfxmode=1056x400x16      # 132x25 @ 8x16 font
    insmod vbe                    # Load VESA BIOS Extensions module
    multiboot /boot/hueos.bin res=132x25
    boot
}
```

Pixel dimensions are calculated as:
- 132 columns × 8 pixels/char = 1056 pixels wide
- 25 rows × 16 pixels/char = 400 pixels high
- Similar calculations for other modes

## How It Works

1. **Boot**: GRUB loads and parses its configuration
2. **VESA Mode Set**: GRUB calls VESA BIOS (INT 10h) in real mode to set the requested graphics mode
3. **Mode Switch**: GRUB switches to protected mode and loads the kernel
4. **Kernel Boot**: Multiboot header tells GRUB to pass video mode info
5. **Detection**: Kernel reads `framebuffer_width` and `framebuffer_height` from multiboot info
6. **Configuration**: Terminal driver updates its dimensions to match the detected mode
7. **Rendering**: VGA text mode buffer at 0xB8000 is now 132 columns wide

## Advantages of This Approach

✅ **No V8086 Mode**: Avoids complex virtual 8086 mode implementation  
✅ **No Real Mode Transitions**: No need for mode switching code  
✅ **Bootloader Support**: Leverages GRUB's tested VESA implementation  
✅ **Platform Independent**: Works across different systems (QEMU, VirtualBox, VMware, real hardware)  
✅ **Clean Separation**: Video mode setup happens before kernel starts  

## Supported Modes

| Mode | Resolution | Pixels | VESA Mode | Status |
|------|-----------|--------|-----------|--------|
| 80x25 | Standard | 640x400 | 0x03 | ✅ Native VGA |
| 80x50 | High-res | 640x400 | Custom | ✅ VGA 400-line + 8x8 font |
| 132x25 | Wide | 1056x400 | 0x109 | ✅ VESA via GRUB |
| 132x43 | Wide high-res | 1056x688 | 0x10A | ✅ VESA via GRUB |
| 132x50 | Wide extra lines | 1056x800 | 0x10B | ✅ VESA via GRUB |

## Testing

Test in QEMU:
```bash
make iso
qemu-system-i386 -cdrom hueos.iso -serial stdio -display gtk
```

At the GRUB menu, select any of the 132x modes. The kernel will:
1. Detect the mode from multiboot info
2. Configure terminal dimensions
3. Render output in the wide format

## Platform Compatibility

- **QEMU**: Full support with `-display gtk` or SDL
- **VirtualBox**: Supports VESA text modes
- **VMware**: Supports VESA text modes
- **Hyper-V**: Limited VESA support, may fall back to 80x25
- **Real Hardware**: Depends on BIOS VESA implementation

## Limitations

1. **Font Control**: Using standard VGA fonts (8x16 or 9x16), cannot easily customize for wide modes
2. **Color Depth**: Limited to 16-color text mode (4-bit color)
3. **BIOS Dependency**: Requires VESA-compliant BIOS
4. **No Runtime Switching**: Mode must be set at boot time (no dynamic switching from kernel)

## Future Enhancements

Possible improvements:
- [ ] Load custom fonts for 132-column modes
- [ ] Implement mode switching via V8086 mode for runtime changes
- [ ] Add more exotic modes (100x37, 160x50, etc.)
- [ ] Graphics mode support for GUI console
- [ ] Framebuffer console for non-text VESA modes

## Files Modified

- `boot/multiboot.asm`: Added video mode fields to multiboot header
- `include/kernel.h`: Expanded multiboot_info structure with framebuffer fields
- `kernel/vesa.c`: Added init_vesa_with_mbi() for multiboot video detection
- `kernel/main.c`: Use init_vesa_with_mbi() instead of init_vesa()
- `Makefile`: Updated GRUB config to set gfxmode for 132x modes

## References

- Multiboot Specification: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
- VESA BIOS Extensions: https://en.wikipedia.org/wiki/VESA_BIOS_Extensions
- GRUB Manual (Video Modes): https://www.gnu.org/software/grub/manual/grub/html_node/Video-modes.html
- VGA Text Mode: https://wiki.osdev.org/Text_Mode_Cursor

## Conclusion

By leveraging GRUB's VESA support and the multiboot video specification, HueOS now fully supports 132-column text modes without requiring complex real-mode BIOS calls or V8086 mode implementation. The solution is clean, maintainable, and works across multiple platforms.
