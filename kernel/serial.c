#include "serial.h"

static int serial_initialized = 0;

void init_serial(void) {
    outb(COM1 + 1, 0x00);    // Disable all interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1 + 1, 0x00);    //                  (hi byte)
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    outb(COM1 + 4, 0x1E);    // Set in loopback mode, test the serial chip
    outb(COM1 + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    
    // Check if serial is faulty (i.e: not same byte as sent)
    if(inb(COM1 + 0) != 0xAE) {
        serial_initialized = 0;
        return;
    }
    
    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(COM1 + 4, 0x0F);
    serial_initialized = 1;
}

static int is_transmit_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_writechar(char c) {
    if (!serial_initialized) return;
    
    while (is_transmit_empty() == 0);
    outb(COM1, c);
}

void serial_write(const char* data) {
    if (!serial_initialized) return;
    
    while (*data) {
        if (*data == '\n') {
            serial_writechar('\r');
        }
        serial_writechar(*data++);
    }
}

void serial_write_hex(uint32_t value) {
    if (!serial_initialized) return;
    
    const char hex_chars[] = "0123456789ABCDEF";
    char buffer[9];
    buffer[8] = '\0';
    
    for (int i = 7; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    serial_write(buffer);
}