# HueOS Display Modes

HueOS supports multiple text display resolutions to give you more screen real estate or better readability.

## Available Text Modes

### Standard Resolutions

| Resolution | Columns | Rows | Description |
|------------|---------|------|-------------|
| **80x25** | 80 | 25 | Standard VGA text mode (default) |
| **80x50** | 80 | 50 | High resolution - double the lines |

### Wide Screen Modes

| Resolution | Columns | Rows | Description |
|------------|---------|------|-------------|
| **132x25** | 132 | 25 | Wide screen - more columns |
| **132x43** | 132 | 43 | Wide high resolution |
| **132x50** | 132 | 50 | Extra wide with many lines |

### Combined Modes

| Resolution | Description |
|------------|-------------|
| **100x37** | Balanced wide mode |

## GRUB Boot Menu Options

When you boot HueOS, you'll see several options in the GRUB menu:

1. **HueOS** - Standard 80x25 mode
2. **HueOS (Verbose - Detailed Hardware Info)** - Standard resolution with verbose output
3. **HueOS (80x50 High Resolution)** - Double the vertical lines
4. **HueOS (132x25 Wide Screen)** - Wide screen mode
5. **HueOS (132x43 Wide High Res)** - Wide with more lines
6. **HueOS (132x50 Extra Lines)** - Maximum screen space
7. **HueOS (Verbose + 80x50 High Res)** - Verbose mode with high resolution

## Using Different Resolutions

### From GRUB Menu

Simply use the arrow keys to select your desired resolution from the boot menu and press Enter.

### From Command Line

You can also specify a resolution directly by editing the GRUB command line (press 'e' at the boot menu):

```
multiboot /boot/hueos.bin res=132x43
```

Replace `132x43` with any supported resolution.

### Combining with Verbose Mode

You can use both verbose mode and a custom resolution:

```
multiboot /boot/hueos.bin verbose res=132x50
```

## How It Works

HueOS uses several techniques to support different resolutions:

1. **VGA Hardware Mode Switching**: For supported modes (80x25, 80x50), the kernel directly programs VGA registers
2. **VESA Text Mode Detection**: The kernel reads VGA CRTC registers to detect the current screen dimensions
3. **Command Line Parsing**: Resolution parameters from GRUB are parsed and applied
4. **Dynamic Terminal**: The terminal driver adapts to different screen sizes
5. **GRUB gfxpayload**: Uses GRUB's text mode payload to maintain compatibility

### Supported Hardware Modes

**✅ Fully Implemented (Hardware + Software):**
- **80x25** - Standard VGA mode (always available)
- **80x50** - 400-line VGA mode with 8-pixel font (works on most hardware)

**⚠️ Software Only (Limited Support):**
- **132x25, 132x43, 132x50** - These modes require VESA BIOS support
- Software terminal dimensions are updated but hardware mode may not change
- These work best when BIOS/firmware sets the mode before kernel boot
- May appear as 80x25 on hardware without VESA support

## Technical Details

### CRTC Register Detection

The kernel reads these VGA CRTC registers to auto-detect resolution:

- **0x12**: Vertical Display End (VDE) - lower 8 bits
- **0x07**: Overflow register - contains VDE bits 8 and 9
- **0x01**: Horizontal Display End (HDE)
- **0x09**: Maximum Scan Line (character height)

From these values, the actual screen dimensions are calculated:

```
width = HDE + 1
height = (VDE + 1) / character_height
```

### Resolution Validation

Detected dimensions must be within reasonable ranges:
- Width: 40 to 200 columns
- Height: 20 to 100 rows

If detection fails, the kernel falls back to standard 80x25 mode.

### Serial Output

Resolution detection information is always logged to the serial port (COM1):

```
VESA: Initializing video mode support
VESA: Detected screen dimensions: 132x43
```

## Platform Support

### Hyper-V

Most text resolutions work in Hyper-V Generation 1 VMs. Note:
- Use Gen1 VMs (BIOS boot) not Gen2 (UEFI boot)
- Some resolutions may fall back to 80x25 depending on VM settings

### QEMU

All resolutions are supported in QEMU. For best results:

```bash
qemu-system-i386 -cdrom hueos.iso
```

### VirtualBox

VirtualBox supports most text modes. Configure:
- Display > Graphics Controller: VBoxVGA or VBoxSVGA
- Enable 3D Acceleration: Optional

### VMware

VMware Workstation and Player support all standard text modes.

### Physical Hardware

Physical hardware support depends on your graphics card:
- Standard VGA modes (80x25, 80x50) work on all hardware
- VESA text modes (132x25, etc.) require VESA BIOS support
- Modern UEFI-only systems may not support all modes

## Troubleshooting

### Issue: Screen appears blank or corrupted

**Solution**: Try a different resolution or use standard 80x25 mode. Some VMs may not support all VESA text modes.

### Issue: Text is cut off on the right side

**Solution**: The terminal detected an incorrect width. This may happen in some VMs. Try:
1. Using standard 80x25 mode
2. Adjusting VM display settings
3. Checking serial output for actual detected dimensions

### Issue: Resolution specified but not applied

**Solution**: 
1. Verify you're using the correct syntax: `res=WIDTHxHEIGHT`
2. Check serial output to see what was detected
3. Ensure GRUB has `set gfxpayload=text` before the multiboot line

### Issue: Want to add a custom resolution

**Solution**: Edit `/boot/grub/grub.cfg` in the ISO or modify the Makefile:

```bash
echo 'menuentry "HueOS (Custom)" {' >> grub.cfg
echo '    set gfxpayload=text' >> grub.cfg
echo '    multiboot /boot/hueos.bin res=100x37' >> grub.cfg
echo '    boot' >> grub.cfg
echo '}' >> grub.cfg
```

Then rebuild with `make iso`.

## Supported Resolutions Reference

The kernel supports these pre-defined text modes:

```c
static const text_mode_t text_modes[] = {
    {"80x25",   80,  25, 0x03},   // Standard VGA
    {"80x50",   80,  50, 0x01},   // VGA 80x50
    {"100x37",  100, 37, 0x6A},   // VESA 100x37
    {"132x25",  132, 25, 0x109},  // VESA 132x25
    {"132x43",  132, 43, 0x10A},  // VESA 132x43
    {"132x50",  132, 50, 0x10B},  // VESA 132x50
};
```

To add more modes, edit `kernel/vesa.c` and rebuild.

## Benefits of Higher Resolutions

### Development and Debugging
- More lines visible for logs and debug output
- Wider display for memory dumps and registers
- Better for verbose hardware information display

### System Monitoring
- More space for system statistics
- Multiple columns of information
- Detailed process listings

### User Interface
- Potential for split-screen interfaces
- More readable text layouts
- Better organization of information

## Future Enhancements

Planned improvements for display support:

- [ ] Graphical framebuffer mode (VBE 2.0+)
- [ ] Font loading support
- [ ] Multiple virtual consoles
- [ ] Color scheme customization
- [ ] Unicode/UTF-8 text rendering
- [ ] Hardware cursor support
- [ ] Smooth scrolling

## Related Documentation

- [Boot Issue Resolution](BOOT_ISSUE_RESOLUTION.md) - Troubleshooting boot problems
- [Hardware Info](IDE_AND_HARDWARE.md) - Benefits of verbose mode with higher resolution
- [Quick Start](QUICK_START_HYPERV.md) - Getting started with HueOS

## API Reference

For kernel developers working on display features:

### Terminal Functions

```c
// Set terminal dimensions dynamically
void terminal_set_dimensions(size_t width, size_t height);

// Get current terminal dimensions
void terminal_get_dimensions(size_t* width, size_t* height);
```

### VESA Functions

```c
// Initialize VESA mode support
void init_vesa(const char* cmdline);

// Parse resolution from command line
int parse_resolution(const char* cmdline);

// Detect current screen size from hardware
void detect_screen_size(void);

// Print all available modes
void print_available_modes(void);
```

## Example Output

### Standard 80x25 Mode
```
HueOS Kernel Starting...
========================
Multiboot loader detected
Initializing GDT...
GDT initialized
...
```

### High Resolution 132x50 Mode
```
HueOS Kernel Starting...
=========================================================================
Multiboot loader detected
Initializing GDT...
GDT initialized
Initializing IDT...
IDT initialized
...
[More lines visible on screen]
...
```

The higher resolution modes give you much more screen real estate for system information and debugging output.
