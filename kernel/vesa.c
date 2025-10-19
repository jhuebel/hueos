#include "kernel.h"
#include "font8x8.h"

// VGA register ports
#define VGA_MISC_WRITE      0x3C2
#define VGA_MISC_READ       0x3CC
#define VGA_SEQ_INDEX       0x3C4
#define VGA_SEQ_DATA        0x3C5
#define VGA_CRTC_INDEX      0x3D4
#define VGA_CRTC_DATA       0x3D5
#define VGA_GC_INDEX        0x3CE
#define VGA_GC_DATA         0x3CF
#define VGA_AC_INDEX        0x3C0
#define VGA_AC_READ         0x3C1
#define VGA_INSTAT_READ     0x3DA

// VESA mode structure
struct vesa_mode_info {
    uint16_t attributes;
    uint8_t  window_a;
    uint8_t  window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t  w_char;
    uint8_t  y_char;
    uint8_t  planes;
    uint8_t  bpp;
    uint8_t  banks;
    uint8_t  memory_model;
    uint8_t  bank_size;
    uint8_t  image_pages;
    uint8_t  reserved0;
} __attribute__((packed));

// VGA mode switching functions
void vga_write_regs(const uint8_t *regs, size_t count, uint16_t port_index, uint16_t port_data) {
    for (size_t i = 0; i < count; i++) {
        outb(port_index, i);
        outb(port_data, regs[i]);
    }
}

// Reverse bits in a byte (for VGA font bit order)
static inline uint8_t reverse_bits(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

// Load 8x8 VGA font from embedded data
void load_8x8_font(void) {
    serial_write("VESA: Loading 8x8 font\n");
    
    // Save sequencer Map Mask register
    outb(VGA_SEQ_INDEX, 0x02);
    uint8_t old_map_mask = inb(VGA_SEQ_DATA);
    
    // Save Graphics Controller Mode register
    outb(VGA_GC_INDEX, 0x05);
    uint8_t old_gc_mode = inb(VGA_GC_DATA);
    
    // Save Graphics Controller Misc register
    outb(VGA_GC_INDEX, 0x06);
    uint8_t old_gc_misc = inb(VGA_GC_DATA);
    
    // Set sequencer to write to plane 2 (font plane)
    outb(VGA_SEQ_INDEX, 0x02);
    outb(VGA_SEQ_DATA, 0x04);  // Enable writes to plane 2
    
    // Set sequencer memory mode - sequential addressing
    outb(VGA_SEQ_INDEX, 0x04);
    outb(VGA_SEQ_DATA, 0x07);  // Sequential mode, extended memory
    
    // Set Graphics Controller mode - write mode 0, read mode 0
    outb(VGA_GC_INDEX, 0x05);
    outb(VGA_GC_DATA, 0x00);
    
    // Set Graphics Controller misc - map to 0xA0000, odd/even disable
    outb(VGA_GC_INDEX, 0x06);
    outb(VGA_GC_DATA, 0x00);  // Map to 0xA0000
    
    // Copy 8x8 font from embedded data to VGA memory
    uint8_t* vga_font = (uint8_t*)0xA0000;
    
    // Copy first 128 characters from embedded font
    // VGA expects MSB as leftmost pixel, so reverse bits
    for (int ch = 0; ch < 128; ch++) {
        for (int row = 0; row < 8; row++) {
            vga_font[ch * 32 + row] = reverse_bits(font_8x8[ch][row]);
        }
    }
    
    // Copy again for extended ASCII (128-255)
    for (int ch = 128; ch < 256; ch++) {
        for (int row = 0; row < 8; row++) {
            // Use character 0 (blank) for extended ASCII
            vga_font[ch * 32 + row] = reverse_bits(font_8x8[0][row]);
        }
    }
    
    // Restore registers
    outb(VGA_SEQ_INDEX, 0x02);
    outb(VGA_SEQ_DATA, old_map_mask);
    
    outb(VGA_GC_INDEX, 0x05);
    outb(VGA_GC_DATA, old_gc_mode);
    
    outb(VGA_GC_INDEX, 0x06);
    outb(VGA_GC_DATA, old_gc_misc);
    
    // Reset sequencer for normal text mode
    outb(VGA_SEQ_INDEX, 0x04);
    outb(VGA_SEQ_DATA, 0x03);  // Normal text mode
    
    serial_write("VESA: 8x8 font loaded\n");
}

// Set 80x50 text mode (400-line mode with 8-pixel font)
void set_80x50_mode(void) {
    serial_write("VESA: Switching to 80x50 text mode\n");
    
    // Unlock CRTC registers (must do this first!)
    outb(VGA_CRTC_INDEX, 0x11);
    uint8_t crtc11 = inb(VGA_CRTC_DATA);
    outb(VGA_CRTC_DATA, crtc11 & 0x7F);  // Clear bit 7 to unlock
    
    // Read misc output register to ensure we're in correct mode
    uint8_t misc = inb(VGA_MISC_READ);
    misc |= 0x01;  // Ensure I/O address select (color mode)
    outb(VGA_MISC_WRITE, misc);
    
    // Load 8x8 font before changing display parameters
    load_8x8_font();
    
    // Set Maximum Scan Line register for 8-pixel characters (instead of 16)
    outb(VGA_CRTC_INDEX, 0x09);
    uint8_t maxscan = inb(VGA_CRTC_DATA);
    maxscan &= 0xE0;  // Clear lower 5 bits
    maxscan |= 0x07;  // Set to 7 (8 scan lines per char: 0-7)
    outb(VGA_CRTC_DATA, maxscan);
    
    // Set Cursor Start register
    outb(VGA_CRTC_INDEX, 0x0A);
    outb(VGA_CRTC_DATA, 0x06);  // Cursor starts at line 6
    
    // Set Cursor End register
    outb(VGA_CRTC_INDEX, 0x0B);
    outb(VGA_CRTC_DATA, 0x07);  // Cursor ends at line 7
    
    // Set Vertical Total (0x06) - must be set before VDE
    outb(VGA_CRTC_INDEX, 0x06);
    outb(VGA_CRTC_DATA, 0xBF);  // 447 total scan lines
    
    // Set Overflow register (contains high bits of vertical params)
    outb(VGA_CRTC_INDEX, 0x07);
    uint8_t overflow = inb(VGA_CRTC_DATA);
    overflow &= 0x10;  // Keep only bit 4
    overflow |= 0x01;  // VT bit 8
    overflow |= 0x02;  // VDE bit 8
    overflow |= 0x20;  // VBS bit 8
    outb(VGA_CRTC_DATA, overflow);
    
    // Set Vertical Display End (0x12) - 400 lines (0x18F)
    outb(VGA_CRTC_INDEX, 0x12);
    outb(VGA_CRTC_DATA, 0x8F);  // Lower 8 bits of 399
    
    // Set Vertical Blank Start (0x15)
    outb(VGA_CRTC_INDEX, 0x15);
    outb(VGA_CRTC_DATA, 0x8F);  // Start blanking at line 399
    
    // Set Vertical Blank End (0x16)
    outb(VGA_CRTC_INDEX, 0x16);
    outb(VGA_CRTC_DATA, 0xB9);  // End blanking at appropriate line
    
    // Set Vertical Retrace Start
    outb(VGA_CRTC_INDEX, 0x10);
    outb(VGA_CRTC_DATA, 0x9C);
    
    // Set Vertical Retrace End
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, 0x8E);
    
    // Line Compare register
    outb(VGA_CRTC_INDEX, 0x18);
    outb(VGA_CRTC_DATA, 0xFF);
    
    serial_write("VESA: 80x50 mode set (400-line, 8-pixel font)\n");
}

// Text mode resolution options
typedef struct {
    const char* name;
    size_t width;
    size_t height;
    uint16_t vesa_mode;
} text_mode_t;

static const text_mode_t text_modes[] = {
    {"80x25",   80,  25, 0x03},   // Standard VGA
    {"80x50",   80,  50, 0x01},   // VGA 80x50
    {"100x37",  100, 37, 0x6A},   // VESA 100x37
    {"132x25",  132, 25, 0x109},  // VESA 132x25
    {"132x43",  132, 43, 0x10A},  // VESA 132x43
    {"132x50",  132, 50, 0x10B},  // VESA 132x50
    {NULL, 0, 0, 0}
};

// Try to detect actual screen dimensions by checking VGA registers
void detect_screen_size(void) {
    // Read CRTC registers to determine actual screen dimensions
    // CRTC Address Register is at 0x3D4 (color) or 0x3B4 (mono)
    // We'll use color mode (0x3D4)
    
    uint16_t crtc_addr = 0x3D4;
    uint16_t crtc_data = 0x3D5;
    
    // Register 0x12: Vertical Display End
    outb(crtc_addr, 0x12);
    uint8_t vde_low = inb(crtc_data);
    
    // Register 0x07: Overflow register (contains VDE bit 8 and 9)
    outb(crtc_addr, 0x07);
    uint8_t overflow = inb(crtc_data);
    
    // Bits 1 and 6 of overflow contain bits 8 and 9 of VDE
    uint16_t vertical_display_end = vde_low | 
                                    ((overflow & 0x02) << 7) | 
                                    ((overflow & 0x40) << 3);
    
    // Register 0x01: Horizontal Display End
    outb(crtc_addr, 0x01);
    uint8_t hde = inb(crtc_data);
    
    // Register 0x09: Maximum Scan Line (character height - 1)
    outb(crtc_addr, 0x09);
    uint8_t max_scan_line = inb(crtc_data) & 0x1F;
    
    size_t char_height = max_scan_line + 1;
    size_t char_width = 8;  // Usually 8 or 9, but 8 is most common
    
    // Calculate dimensions
    size_t width = hde + 1;
    size_t height = (vertical_display_end + 1) / char_height;
    
    // Validate dimensions (must be reasonable)
    if (width >= 40 && width <= 200 && height >= 20 && height <= 100) {
        terminal_set_dimensions(width, height);
        
        serial_write("VESA: Detected screen dimensions: ");
        // Simple number to string conversion
        char buf[16];
        int i = 0;
        size_t temp = width;
        do {
            buf[i++] = '0' + (temp % 10);
            temp /= 10;
        } while (temp > 0);
        // Reverse the string
        for (int j = 0; j < i/2; j++) {
            char t = buf[j];
            buf[j] = buf[i-1-j];
            buf[i-1-j] = t;
        }
        buf[i] = 'x';
        i++;
        temp = height;
        int start = i;
        do {
            buf[i++] = '0' + (temp % 10);
            temp /= 10;
        } while (temp > 0);
        // Reverse the height part
        for (int j = start; j < (start + (i - start)/2); j++) {
            char t = buf[j];
            buf[j] = buf[i-1-(j-start)];
            buf[i-1-(j-start)] = t;
        }
        buf[i] = '\n';
        buf[i+1] = '\0';
        serial_write(buf);
    } else {
        serial_write("VESA: Could not detect valid screen dimensions, using 80x25\n");
        terminal_set_dimensions(80, 25);
    }
}

// Parse resolution from kernel command line
int parse_resolution(const char* cmdline) {
    if (!cmdline) return 0;
    
    // Look for "res=" or "resolution=" parameter
    const char* res_param = NULL;
    const char* p = cmdline;
    
    while (*p) {
        if ((p[0] == 'r' && p[1] == 'e' && p[2] == 's' && p[3] == '=')) {
            res_param = p + 4;
            break;
        }
        if ((p[0] == 'r' && p[1] == 'e' && p[2] == 's' && p[3] == 'o' && 
             p[4] == 'l' && p[5] == 'u' && p[6] == 't' && p[7] == 'i' && 
             p[8] == 'o' && p[9] == 'n' && p[10] == '=')) {
            res_param = p + 11;
            break;
        }
        p++;
    }
    
    if (!res_param) return 0;
    
    // Try to match against known modes
    for (int i = 0; text_modes[i].name != NULL; i++) {
        const char* mode_name = text_modes[i].name;
        const char* param = res_param;
        int match = 1;
        
        while (*mode_name && *param && *param != ' ') {
            if (*mode_name != *param) {
                match = 0;
                break;
            }
            mode_name++;
            param++;
        }
        
        if (match && *mode_name == '\0') {
            // Found matching mode - now actually set the VGA hardware mode
            serial_write("VESA: Requested resolution: ");
            serial_write(text_modes[i].name);
            serial_write("\n");
            
            // Switch hardware mode based on resolution
            if (text_modes[i].width == 80 && text_modes[i].height == 50) {
                set_80x50_mode();
            } else if (text_modes[i].width == 80 && text_modes[i].height == 25) {
                // Already in 80x25, just ensure it
                serial_write("VESA: Already in 80x25 mode\n");
            } else {
                // For other modes, we can't easily set them without BIOS
                // Just update software dimensions and warn
                serial_write("VESA: Warning - Cannot set hardware mode for ");
                serial_write(text_modes[i].name);
                serial_write(", using software dimensions only\n");
            }
            
            terminal_set_dimensions(text_modes[i].width, text_modes[i].height);
            return 1;
        }
    }
    
    return 0;
}

void init_vesa(const char* cmdline) {
    serial_write("VESA: Initializing video mode support\n");
    
    // First, try to parse resolution from command line
    if (parse_resolution(cmdline)) {
        return;  // Successfully set from command line
    }
    
    // Otherwise, try to detect current mode
    detect_screen_size();
}

// Initialize VESA with multiboot video info
void init_vesa_with_mbi(const char* cmdline, struct multiboot_info* mbi) {
    serial_write("VESA: Initializing video mode support\n");
    
    // Check if multiboot provided framebuffer info (flag bit 12)
    if (mbi && (mbi->flags & (1 << 12))) {
        serial_write("VESA: Multiboot framebuffer info available\n");
        
        // For text mode, framebuffer_type should be 2
        if (mbi->framebuffer_type == 2) {
            serial_write("VESA: Text mode detected from multiboot\n");
            size_t width = mbi->framebuffer_width;
            size_t height = mbi->framebuffer_height;
            
            // Validate dimensions
            if (width >= 40 && width <= 200 && height >= 20 && height <= 100) {
                terminal_set_dimensions(width, height);
                
                // If we're in 80x50, we need to load the 8x8 font
                if (width == 80 && height == 50) {
                    load_8x8_font();
                }
                
                serial_write("VESA: Set dimensions from multiboot: ");
                char buf[32];
                int i = 0;
                size_t temp = width;
                do {
                    buf[i++] = '0' + (temp % 10);
                    temp /= 10;
                } while (temp > 0);
                for (int j = 0; j < i/2; j++) {
                    char t = buf[j];
                    buf[j] = buf[i-1-j];
                    buf[i-1-j] = t;
                }
                buf[i++] = 'x';
                int start = i;
                temp = height;
                do {
                    buf[i++] = '0' + (temp % 10);
                    temp /= 10;
                } while (temp > 0);
                for (int j = start; j < (start + (i - start)/2); j++) {
                    char t = buf[j];
                    buf[j] = buf[i-1-(j-start)];
                    buf[i-1-(j-start)] = t;
                }
                buf[i++] = '\n';
                buf[i] = '\0';
                serial_write(buf);
                return;
            }
        }
    }
    
    // Fall back to command line or detection
    init_vesa(cmdline);
}

void print_available_modes(void) {
    terminal_writestring("Available text modes:\n");
    serial_write("Available text modes:\n");
    
    for (int i = 0; text_modes[i].name != NULL; i++) {
        terminal_writestring("  - ");
        terminal_writestring(text_modes[i].name);
        terminal_writestring("\n");
        
        serial_write("  - ");
        serial_write(text_modes[i].name);
        serial_write("\n");
    }
    
    terminal_writestring("\nTo use a specific mode, add to GRUB command line: res=WIDTHxHEIGHT\n");
    terminal_writestring("Example: res=132x43\n\n");
    serial_write("\nTo use a specific mode, add to GRUB command line: res=WIDTHxHEIGHT\n");
}
