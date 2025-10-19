#include "ide.h"
#include "kernel.h"

static ide_device_t ide_devices[4];
static int device_count = 0;

// Channel information
static struct {
    uint16_t base;
    uint16_t ctrl;
} channels[2] = {
    {ATA_PRIMARY_IO, ATA_PRIMARY_CTRL},
    {ATA_SECONDARY_IO, ATA_SECONDARY_CTRL}
};

// Read from IDE register
static uint8_t ide_read(uint8_t channel, uint8_t reg) {
    uint8_t result;
    if (reg > 0x07 && reg < 0x0C)
        ide_read(channel, ATA_REG_CONTROL);
    
    if (reg < 0x08)
        result = inb(channels[channel].base + reg);
    else if (reg < 0x0C)
        result = inb(channels[channel].base + reg - 0x06);
    else if (reg < 0x0E)
        result = inb(channels[channel].ctrl + reg - 0x0A);
    else if (reg < 0x16)
        result = inb(channels[channel].base + reg - 0x0E);
    
    return result;
}

// Write to IDE register
static void ide_write(uint8_t channel, uint8_t reg, uint8_t data) {
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].ctrl);
    
    if (reg < 0x08)
        outb(channels[channel].base + reg, data);
    else if (reg < 0x0C)
        outb(channels[channel].base + reg - 0x06, data);
    else if (reg < 0x0E)
        outb(channels[channel].ctrl + reg - 0x0A, data);
    else if (reg < 0x16)
        outb(channels[channel].base + reg - 0x0E, data);
}

// Read buffer from IDE
static void ide_read_buffer(uint8_t channel, uint8_t reg, uint16_t* buffer, uint32_t count) {
    if (reg > 0x07 && reg < 0x0C)
        ide_read(channel, ATA_REG_CONTROL);
    
    if (reg < 0x08) {
        for (uint32_t i = 0; i < count; i++)
            buffer[i] = inw(channels[channel].base + reg);
    }
}

// Wait for IDE device
static uint8_t ide_polling(uint8_t channel, uint8_t advanced_check) {
    // Wait 400ns
    for (int i = 0; i < 4; i++)
        ide_read(channel, ATA_REG_ALTSTATUS);
    
    // Wait for BSY to be cleared
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);
    
    if (advanced_check) {
        uint8_t state = ide_read(channel, ATA_REG_STATUS);
        
        if (state & ATA_SR_ERR)
            return 2; // Error
        
        if (state & ATA_SR_DF)
            return 1; // Device fault
        
        if (!(state & ATA_SR_DRQ))
            return 3; // DRQ not set
    }
    
    return 0; // No error
}

// String helper for model names
static void ide_string_copy(char* dest, uint16_t* src, int length) {
    for (int i = 0; i < length; i += 2) {
        dest[i] = (src[i / 2] >> 8) & 0xFF;
        dest[i + 1] = src[i / 2] & 0xFF;
    }
    dest[length] = '\0';
    
    // Trim trailing spaces
    for (int i = length - 1; i >= 0; i--) {
        if (dest[i] == ' ')
            dest[i] = '\0';
        else
            break;
    }
}

void init_ide(void) {
    serial_write("Initializing IDE controllers...\n");
    device_count = 0;
    
    // Disable interrupts
    ide_write(0, ATA_REG_CONTROL, 2);
    ide_write(1, ATA_REG_CONTROL, 2);
    
    serial_write("IDE controllers initialized\n");
}

void ide_detect_devices(void) {
    uint16_t identify_buffer[256];
    
    serial_write("Detecting IDE devices...\n");
    
    for (uint8_t channel = 0; channel < 2; channel++) {
        for (uint8_t drive = 0; drive < 2; drive++) {
            uint8_t err = 0;
            uint8_t type = IDE_ATA;
            
            ide_devices[device_count].reserved = 0;
            ide_devices[device_count].channel = channel;
            ide_devices[device_count].drive = drive;
            
            // Select drive
            ide_write(channel, ATA_REG_HDDEVSEL, 0xA0 | (drive << 4));
            
            // Wait 1ms
            for (int i = 0; i < 1000; i++)
                inb(channels[channel].ctrl);
            
            // Send IDENTIFY command
            ide_write(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            
            // Wait 1ms
            for (int i = 0; i < 1000; i++)
                inb(channels[channel].ctrl);
            
            // Check if device exists
            if (ide_read(channel, ATA_REG_STATUS) == 0) {
                continue; // No device
            }
            
            // Poll
            err = ide_polling(channel, 0);
            
            // Check for ATAPI
            uint8_t cl = ide_read(channel, ATA_REG_LBA1);
            uint8_t ch = ide_read(channel, ATA_REG_LBA2);
            
            if ((cl == 0x14 && ch == 0xEB) || (cl == 0x69 && ch == 0x96)) {
                type = IDE_ATAPI;
                ide_write(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                for (int i = 0; i < 1000; i++)
                    inb(channels[channel].ctrl);
            } else if (cl != 0 || ch != 0 || err) {
                continue; // Unknown type or error
            }
            
            // Read identification space
            ide_read_buffer(channel, ATA_REG_DATA, identify_buffer, 256);
            
            // Fill device structure
            ide_devices[device_count].type = type;
            ide_devices[device_count].signature = *((uint16_t*)(identify_buffer + 0));
            ide_devices[device_count].capabilities = *((uint16_t*)(identify_buffer + 49));
            ide_devices[device_count].command_sets = *((uint32_t*)(identify_buffer + 82));
            
            // Get size
            if (ide_devices[device_count].command_sets & (1 << 26)) {
                ide_devices[device_count].size = *((uint32_t*)(identify_buffer + 60));
            } else {
                ide_devices[device_count].size = 0;
            }
            
            // Get model
            ide_string_copy(ide_devices[device_count].model, identify_buffer + 27, 40);
            
            device_count++;
        }
    }
    
    serial_write("IDE device detection complete\n");
}

uint8_t ide_read_sector(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t* buffer) {
    uint8_t lba_mode, lba_io[6];
    
    // LBA28 mode
    lba_mode = 0;
    lba_io[0] = (lba & 0x000000FF) >> 0;
    lba_io[1] = (lba & 0x0000FF00) >> 8;
    lba_io[2] = (lba & 0x00FF0000) >> 16;
    lba_io[3] = 0;
    lba_io[4] = 0;
    lba_io[5] = 0;
    
    // Wait for drive to be ready
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);
    
    // Select drive
    ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    
    // Write parameters
    ide_write(channel, ATA_REG_SECCOUNT0, 1);
    ide_write(channel, ATA_REG_LBA0, lba_io[0]);
    ide_write(channel, ATA_REG_LBA1, lba_io[1]);
    ide_write(channel, ATA_REG_LBA2, lba_io[2]);
    
    // Send command
    ide_write(channel, ATA_REG_COMMAND, ATA_CMD_READ_PIO);
    
    // Poll
    if (ide_polling(channel, 1))
        return 1; // Error
    
    // Read data
    uint16_t* buf16 = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++)
        buf16[i] = inw(channels[channel].base + ATA_REG_DATA);
    
    return 0; // Success
}

uint8_t ide_write_sector(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t* buffer) {
    uint8_t lba_io[6];
    
    // LBA28 mode
    lba_io[0] = (lba & 0x000000FF) >> 0;
    lba_io[1] = (lba & 0x0000FF00) >> 8;
    lba_io[2] = (lba & 0x00FF0000) >> 16;
    lba_io[3] = 0;
    lba_io[4] = 0;
    lba_io[5] = 0;
    
    // Wait for drive to be ready
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);
    
    // Select drive
    ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    
    // Write parameters
    ide_write(channel, ATA_REG_SECCOUNT0, 1);
    ide_write(channel, ATA_REG_LBA0, lba_io[0]);
    ide_write(channel, ATA_REG_LBA1, lba_io[1]);
    ide_write(channel, ATA_REG_LBA2, lba_io[2]);
    
    // Send command
    ide_write(channel, ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
    
    // Poll
    if (ide_polling(channel, 1))
        return 1; // Error
    
    // Write data
    uint16_t* buf16 = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++)
        outw(channels[channel].base + ATA_REG_DATA, buf16[i]);
    
    // Flush cache
    ide_write(channel, ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    ide_polling(channel, 0);
    
    return 0; // Success
}

void ide_print_devices(void) {
    terminal_writestring("\nIDE Devices:\n");
    terminal_writestring("============\n");
    serial_write("\nIDE Devices:\n");
    
    if (device_count == 0) {
        terminal_writestring("No IDE devices detected\n");
        serial_write("No IDE devices detected\n");
        return;
    }
    
    for (int i = 0; i < device_count; i++) {
        ide_device_t* dev = &ide_devices[i];
        
        // Channel and drive info
        terminal_writestring("Device ");
        terminal_putchar('0' + i);
        terminal_writestring(": ");
        
        if (dev->channel == 0)
            terminal_writestring("Primary ");
        else
            terminal_writestring("Secondary ");
        
        if (dev->drive == 0)
            terminal_writestring("Master - ");
        else
            terminal_writestring("Slave - ");
        
        // Type
        if (dev->type == IDE_ATA) {
            terminal_writestring("ATA HDD\n");
            serial_write("  Type: ATA Hard Disk\n");
        } else {
            terminal_writestring("ATAPI CD/DVD\n");
            serial_write("  Type: ATAPI Optical Drive\n");
        }
        
        // Model
        terminal_writestring("  Model: ");
        terminal_writestring(dev->model);
        terminal_writestring("\n");
        
        serial_write("  Model: ");
        serial_write(dev->model);
        serial_write("\n");
        
        // Size (for ATA only)
        if (dev->type == IDE_ATA && dev->size > 0) {
            terminal_writestring("  Size: ");
            // Simple size display (in MB)
            uint32_t size_mb = dev->size / 2048; // 512 byte sectors to MB
            char size_str[16];
            int pos = 0;
            uint32_t temp = size_mb;
            
            if (temp == 0) {
                size_str[pos++] = '0';
            } else {
                char digits[16];
                int d = 0;
                while (temp > 0) {
                    digits[d++] = '0' + (temp % 10);
                    temp /= 10;
                }
                for (int j = d - 1; j >= 0; j--) {
                    size_str[pos++] = digits[j];
                }
            }
            size_str[pos] = '\0';
            
            terminal_writestring(size_str);
            terminal_writestring(" MB\n");
            
            serial_write("  Size: ");
            serial_write(size_str);
            serial_write(" MB\n");
        }
    }
}

int ide_get_device_count(void) {
    return device_count;
}

ide_device_t* ide_get_device(int index) {
    if (index < 0 || index >= device_count)
        return NULL;
    return &ide_devices[index];
}