// Framebuffer text renderer for graphics modes
// Allows 132-column and other arbitrary text resolutions

#include "kernel.h"
#include "font8x8.h"

// Framebuffer state
static uint8_t* fb_address = NULL;
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;
static uint32_t fb_pitch = 0;
static uint8_t fb_bpp = 0;
static uint8_t fb_type = 0;

// Text mode state
static size_t text_cols = 80;
static size_t text_rows = 25;
static size_t cursor_x = 0;
static size_t cursor_y = 0;
static uint8_t text_color = 0x07; // Light gray on black

// Font dimensions
#define FONT_WIDTH 8
#define FONT_HEIGHT 8

// Initialize framebuffer text mode
void framebuffer_init(struct multiboot_info* mbi) {
    // Check if framebuffer info is available
    if (!mbi || !(mbi->flags & (1 << 12))) {
        serial_write("Framebuffer: No multiboot framebuffer info\n");
        return;
    }
    
    fb_type = mbi->framebuffer_type;
    
    // Only initialize for graphics modes (type 1 = RGB graphics)
    // Type 2 = EGA text, which should use normal VGA text mode
    if (fb_type != 1) {
        serial_write("Framebuffer: Text mode detected, using VGA driver\n");
        fb_address = NULL;  // Disable framebuffer
        return;
    }
    
    fb_address = (uint8_t*)(uintptr_t)mbi->framebuffer_addr;
    fb_width = mbi->framebuffer_width;
    fb_height = mbi->framebuffer_height;
    fb_pitch = mbi->framebuffer_pitch;
    fb_bpp = mbi->framebuffer_bpp;
    
    serial_write("Framebuffer: Graphics mode initialized\n");
    serial_write("Framebuffer: Address = 0x");
    char buf[20];
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = ((uintptr_t)fb_address >> (i * 4)) & 0xF;
        buf[7-i] = nibble < 10 ? '0' + nibble : 'a' + nibble - 10;
    }
    buf[8] = '\n';
    buf[9] = '\0';
    serial_write(buf);
    
    // Calculate text dimensions based on font size
    text_cols = fb_width / FONT_WIDTH;
    text_rows = fb_height / FONT_HEIGHT;
    
    serial_write("Framebuffer: Text mode ");
    int idx = 0;
    uint32_t val = text_cols;
    do {
        buf[idx++] = '0' + (val % 10);
        val /= 10;
    } while (val > 0);
    for (int j = 0; j < idx/2; j++) {
        char t = buf[j];
        buf[j] = buf[idx-1-j];
        buf[idx-1-j] = t;
    }
    buf[idx++] = 'x';
    int start = idx;
    val = text_rows;
    do {
        buf[idx++] = '0' + (val % 10);
        val /= 10;
    } while (val > 0);
    for (int j = start; j < (start + (idx - start)/2); j++) {
        char t = buf[j];
        buf[j] = buf[idx-1-(j-start)];
        buf[idx-1-(j-start)] = t;
    }
    buf[idx++] = '\n';
    buf[idx] = '\0';
    serial_write(buf);
    
    // Clear screen
    framebuffer_clear();
}

// Check if framebuffer is initialized
int framebuffer_is_active(void) {
    return (fb_address != NULL && fb_type == 1); // Type 1 = RGB graphics
}

// Get text dimensions
void framebuffer_get_dimensions(size_t* width, size_t* height) {
    if (width) *width = text_cols;
    if (height) *height = text_rows;
}

// Draw a pixel (for 16-bit RGB mode)
static void framebuffer_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!fb_address || x >= fb_width || y >= fb_height) return;
    
    if (fb_bpp == 16) {
        // 16-bit RGB565 or RGB555
        uint16_t* pixel = (uint16_t*)(fb_address + y * fb_pitch + x * 2);
        *pixel = (uint16_t)color;
    } else if (fb_bpp == 32) {
        // 32-bit RGBA
        uint32_t* pixel = (uint32_t*)(fb_address + y * fb_pitch + x * 4);
        *pixel = color;
    }
}

// Convert VGA color to RGB
static uint32_t vga_to_rgb(uint8_t vga_color) {
    // VGA 16-color palette to RGB (simplified)
    static const uint32_t palette[16] = {
        0x0000, 0x0015, 0x0540, 0x0555, // Black, Blue, Green, Cyan
        0xA800, 0xA815, 0xAA80, 0xAD55, // Red, Magenta, Brown, Light Gray
        0x5555, 0x557F, 0x57E0, 0x57FF, // Dark Gray, Light Blue, Light Green, Light Cyan
        0xFD00, 0xFD1F, 0xFFE0, 0xFFFF  // Light Red, Light Magenta, Yellow, White
    };
    
    return palette[vga_color & 0x0F];
}

// Draw a character at specific position
void framebuffer_draw_char(char c, size_t col, size_t row, uint8_t color) {
    if (!fb_address || col >= text_cols || row >= text_rows) return;
    
    uint8_t char_index = (uint8_t)c;
    if (char_index > 127) char_index = 0;
    
    uint32_t fg_color = vga_to_rgb(color & 0x0F);
    uint32_t bg_color = vga_to_rgb((color >> 4) & 0x0F);
    
    uint32_t px = col * FONT_WIDTH;
    uint32_t py = row * FONT_HEIGHT;
    
    for (int y = 0; y < FONT_HEIGHT; y++) {
        uint8_t font_row = font_8x8[char_index][y];
        // Reverse bits for correct display
        font_row = reverse_bits(font_row);
        
        for (int x = 0; x < FONT_WIDTH; x++) {
            uint32_t pixel_color = (font_row & (1 << (7 - x))) ? fg_color : bg_color;
            framebuffer_put_pixel(px + x, py + y, pixel_color);
        }
    }
}

// Scroll screen up by one line
void framebuffer_scroll(void) {
    if (!fb_address) return;
    
    // Copy pixels up by FONT_HEIGHT lines
    for (uint32_t y = FONT_HEIGHT; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            // Copy pixel from (x, y) to (x, y - FONT_HEIGHT)
            if (fb_bpp == 16) {
                uint16_t* src = (uint16_t*)(fb_address + y * fb_pitch + x * 2);
                uint16_t* dst = (uint16_t*)(fb_address + (y - FONT_HEIGHT) * fb_pitch + x * 2);
                *dst = *src;
            } else if (fb_bpp == 32) {
                uint32_t* src = (uint32_t*)(fb_address + y * fb_pitch + x * 4);
                uint32_t* dst = (uint32_t*)(fb_address + (y - FONT_HEIGHT) * fb_pitch + x * 4);
                *dst = *src;
            }
        }
    }
    
    // Clear the last line
    uint32_t bg_color = vga_to_rgb((text_color >> 4) & 0x0F);
    for (uint32_t y = fb_height - FONT_HEIGHT; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            framebuffer_put_pixel(x, y, bg_color);
        }
    }
}

// Clear the screen
void framebuffer_clear(void) {
    if (!fb_address) return;
    
    uint32_t bg_color = vga_to_rgb((text_color >> 4) & 0x0F);
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            framebuffer_put_pixel(x, y, bg_color);
        }
    }
    
    cursor_x = 0;
    cursor_y = 0;
}

// Put a character at cursor position
void framebuffer_putchar(char c) {
    if (!fb_address) return;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
    } else if (c == '\b') {
        if (cursor_x > 0) cursor_x--;
    } else {
        framebuffer_draw_char(c, cursor_x, cursor_y, text_color);
        cursor_x++;
    }
    
    // Handle line wrap
    if (cursor_x >= text_cols) {
        cursor_x = 0;
        cursor_y++;
    }
    
    // Handle scroll
    if (cursor_y >= text_rows) {
        framebuffer_scroll();
        cursor_y = text_rows - 1;
    }
}

// Write string
void framebuffer_writestring(const char* str) {
    if (!fb_address || !str) return;
    
    while (*str) {
        framebuffer_putchar(*str);
        str++;
    }
}

// Set text color
void framebuffer_set_color(uint8_t color) {
    text_color = color;
}
