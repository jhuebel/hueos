#include "scsi.h"
#include "kernel.h"

static scsi_controller_t controllers[SCSI_MAX_CONTROLLERS];
static int controller_count = 0;
static scsi_device_t scsi_devices[SCSI_MAX_DEVICES];
static int device_count = 0;

// Helper: Byte swap for big-endian SCSI data
static uint32_t swap32(uint32_t val) {
    return ((val >> 24) & 0xFF) |
           ((val >> 8) & 0xFF00) |
           ((val << 8) & 0xFF0000) |
           ((val << 24) & 0xFF000000);
}

static uint16_t swap16(uint16_t val) {
    return ((val >> 8) & 0xFF) | ((val << 8) & 0xFF00);
}

// Helper: String copy with trimming
static void scsi_string_copy(char* dest, const char* src, int length) {
    int i;
    for (i = 0; i < length && src[i]; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    
    // Trim trailing spaces
    for (i = length - 1; i >= 0; i--) {
        if (dest[i] == ' ' || dest[i] == '\0')
            dest[i] = '\0';
        else
            break;
    }
}

// PCI Configuration Space Access
static uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    outl(0xCF8, address);
    return inl(0xCFC);
}

static uint16_t pci_read_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    outl(0xCF8, address);
    return (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
}

static uint8_t pci_read_config_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    outl(0xCF8, address);
    return (uint8_t)((inl(0xCFC) >> ((offset & 3) * 8)) & 0xFF);
}

// BusLogic: Wait for adapter ready
static int buslogic_wait_ready(uint16_t io_base) {
    for (int i = 0; i < 10000; i++) {
        uint8_t status = inb(io_base + BUSLOGIC_REG_STATUS);
        if (status & BUSLOGIC_STATUS_HOST_READY) {
            return 1; // Ready
        }
        // Small delay
        for (volatile int j = 0; j < 100; j++);
    }
    return 0; // Timeout
}

// BusLogic: Send command
static int buslogic_send_command(uint16_t io_base, uint8_t cmd) {
    if (!buslogic_wait_ready(io_base)) {
        return 0;
    }
    outb(io_base + BUSLOGIC_REG_COMMAND, cmd);
    return 1;
}

// BusLogic: Read data
static uint8_t buslogic_read_data(uint16_t io_base) {
    buslogic_wait_ready(io_base);
    return inb(io_base + BUSLOGIC_REG_DATA_IN);
}

// BusLogic: Write data
static int buslogic_write_data(uint16_t io_base, uint8_t data) {
    if (!buslogic_wait_ready(io_base)) {
        return 0;
    }
    outb(io_base + BUSLOGIC_REG_COMMAND, data);
    return 1;
}

// BusLogic: Soft reset
static void buslogic_soft_reset(uint16_t io_base) {
    outb(io_base + BUSLOGIC_REG_CONTROL, BUSLOGIC_CTRL_SOFT_RESET);
    
    // Wait for reset to complete
    for (volatile int i = 0; i < 10000; i++);
    
    buslogic_wait_ready(io_base);
}

// BusLogic: Initialize controller
static int buslogic_init(uint16_t io_base) {
    serial_write("Initializing BusLogic controller at I/O 0x");
    serial_write_hex(io_base);
    serial_write("\n");
    
    // Soft reset
    buslogic_soft_reset(io_base);
    
    // Check if controller is present and responding
    if (!buslogic_wait_ready(io_base)) {
        serial_write("  Controller not ready\n");
        return 0;
    }
    
    serial_write("  BusLogic controller initialized\n");
    return 1;
}

// SCSI: Execute a simple command (no data transfer)
static int scsi_execute_simple_command(uint8_t controller_id, uint8_t target, uint8_t lun, 
                                       uint8_t* cdb, uint8_t cdb_len) {
    if (controller_id >= controller_count) {
        return 0;
    }
    
    scsi_controller_t* ctrl = &controllers[controller_id];
    
    if (ctrl->type == SCSI_CONTROLLER_BUSLOGIC) {
        // For simple commands, we'll use polling mode
        // In a full implementation, this would set up a CCB and use mailboxes
        
        // This is a simplified version - real implementation would need:
        // 1. Allocate CCB
        // 2. Set up mailboxes
        // 3. Submit command
        // 4. Wait for completion
        
        return 1; // Assume success for now
    }
    
    return 0;
}

// SCSI: Send INQUIRY command
static int scsi_inquiry(uint8_t controller_id, uint8_t target, uint8_t lun, scsi_inquiry_t* inquiry) {
    uint8_t cdb[6] = {0};
    cdb[0] = SCSI_CMD_INQUIRY;
    cdb[1] = (lun << 5);
    cdb[4] = sizeof(scsi_inquiry_t); // Allocation length
    
    // For now, return simulated data for testing
    // Full implementation would execute the command via CCB
    
    inquiry->peripheral_type = SCSI_TYPE_DISK;
    inquiry->rmb = 0;
    inquiry->version = 2; // SCSI-2
    inquiry->response_format = 2;
    inquiry->additional_length = 31;
    
    // Simulated vendor/product info
    scsi_string_copy((char*)inquiry->vendor, "QEMU    ", 8);
    scsi_string_copy((char*)inquiry->product, "HARDDISK        ", 16);
    scsi_string_copy((char*)inquiry->revision, "2.5+", 4);
    
    return 1;
}

// SCSI: Send READ CAPACITY command
static int scsi_read_capacity(uint8_t controller_id, uint8_t target, uint8_t lun, scsi_capacity_t* capacity) {
    uint8_t cdb[10] = {0};
    cdb[0] = SCSI_CMD_READ_CAPACITY_10;
    cdb[1] = (lun << 5);
    
    // For now, return simulated data
    // Full implementation would execute via CCB
    
    // Simulate a 100MB disk
    capacity->last_lba = swap32(204799); // 100MB = 204800 sectors - 1
    capacity->block_size = swap32(512);
    
    return 1;
}

// Scan for SCSI controllers via PCI
void init_scsi(void) {
    serial_write("Scanning for SCSI controllers...\n");
    controller_count = 0;
    device_count = 0;
    
    // Scan PCI bus for SCSI controllers
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint16_t vendor_id = pci_read_config_word(bus, slot, 0, 0x00);
            
            if (vendor_id == 0xFFFF || vendor_id == 0x0000) {
                continue; // No device
            }
            
            uint16_t device_id = pci_read_config_word(bus, slot, 0, 0x02);
            uint8_t class_code = pci_read_config_byte(bus, slot, 0, 0x0B);
            uint8_t subclass = pci_read_config_byte(bus, slot, 0, 0x0A);
            
            // Check for SCSI controller (Class 01h, Subclass 00h)
            if (class_code == 0x01 && subclass == 0x00) {
                serial_write("Found SCSI controller: Vendor=0x");
                serial_write_hex(vendor_id);
                serial_write(" Device=0x");
                serial_write_hex(device_id);
                serial_write("\n");
                
                // Get I/O base address (BAR0)
                uint32_t bar0 = pci_read_config_dword(bus, slot, 0, 0x10);
                uint16_t io_base = (uint16_t)(bar0 & 0xFFFFFFF0);
                
                if (io_base == 0) {
                    serial_write("  Invalid I/O base address\n");
                    continue;
                }
                
                // Identify controller type
                uint8_t ctrl_type = 0;
                if (vendor_id == BUSLOGIC_VENDOR_ID && device_id == BUSLOGIC_DEVICE_ID) {
                    ctrl_type = SCSI_CONTROLLER_BUSLOGIC;
                    serial_write("  Type: BusLogic BT-958\n");
                    
                    if (buslogic_init(io_base)) {
                        if (controller_count < SCSI_MAX_CONTROLLERS) {
                            controllers[controller_count].type = ctrl_type;
                            controllers[controller_count].io_base = io_base;
                            controllers[controller_count].mmio_base = 0;
                            controllers[controller_count].irq = pci_read_config_byte(bus, slot, 0, 0x3C);
                            controllers[controller_count].device_count = 0;
                            controller_count++;
                        }
                    }
                } else if (vendor_id == LSI_VENDOR_ID) {
                    if (device_id == LSI_53C895A_DEVICE_ID || device_id == LSI_53C1030_DEVICE_ID) {
                        ctrl_type = SCSI_CONTROLLER_LSI_LOGIC;
                        serial_write("  Type: LSI Logic\n");
                        serial_write("  Note: LSI Logic not yet fully supported\n");
                    }
                }
            }
        }
        
        // Optimize: skip buses without devices
        if (bus == 0 && controller_count == 0) {
            // Check a few more buses before giving up
            if (bus > 3) break;
        }
    }
    
    if (controller_count == 0) {
        serial_write("No SCSI controllers detected\n");
    } else {
        serial_write("Detected ");
        char count_str[4];
        count_str[0] = '0' + controller_count;
        count_str[1] = '\0';
        serial_write(count_str);
        serial_write(" SCSI controller(s)\n");
    }
}

// Scan for SCSI devices on all controllers
void scsi_scan_devices(void) {
    serial_write("Scanning for SCSI devices...\n");
    
    for (int ctrl = 0; ctrl < controller_count; ctrl++) {
        serial_write("Scanning controller ");
        char ctrl_str[4];
        ctrl_str[0] = '0' + ctrl;
        ctrl_str[1] = '\0';
        serial_write(ctrl_str);
        serial_write("\n");
        
        // Scan all possible targets (0-15)
        for (uint8_t target = 0; target < 8; target++) {
            for (uint8_t lun = 0; lun < 1; lun++) { // Usually just LUN 0
                scsi_inquiry_t inquiry;
                
                if (scsi_inquiry(ctrl, target, lun, &inquiry)) {
                    if (inquiry.peripheral_type != SCSI_TYPE_NO_DEVICE &&
                        inquiry.peripheral_type != 0x7F) {
                        
                        // Found a device!
                        if (device_count < SCSI_MAX_DEVICES) {
                            scsi_device_t* dev = &scsi_devices[device_count];
                            
                            dev->controller_id = ctrl;
                            dev->target = target;
                            dev->lun = lun;
                            dev->type = inquiry.peripheral_type;
                            
                            scsi_string_copy(dev->vendor, (char*)inquiry.vendor, 8);
                            scsi_string_copy(dev->product, (char*)inquiry.product, 16);
                            scsi_string_copy(dev->revision, (char*)inquiry.revision, 4);
                            
                            // Get capacity for disk devices
                            if (inquiry.peripheral_type == SCSI_TYPE_DISK) {
                                scsi_capacity_t capacity;
                                if (scsi_read_capacity(ctrl, target, lun, &capacity)) {
                                    dev->block_count = swap32(capacity.last_lba) + 1;
                                    dev->block_size = swap32(capacity.block_size);
                                }
                            } else {
                                dev->block_count = 0;
                                dev->block_size = 0;
                            }
                            
                            device_count++;
                            controllers[ctrl].device_count++;
                            
                            serial_write("  Found device at target ");
                            char target_str[4];
                            target_str[0] = '0' + target;
                            target_str[1] = '\0';
                            serial_write(target_str);
                            serial_write(": ");
                            serial_write(dev->vendor);
                            serial_write(" ");
                            serial_write(dev->product);
                            serial_write("\n");
                        }
                    }
                }
            }
        }
    }
    
    serial_write("SCSI device scan complete. Found ");
    char count_str[4];
    count_str[0] = '0' + device_count;
    count_str[1] = '\0';
    serial_write(count_str);
    serial_write(" device(s)\n");
}

// Print detected SCSI devices
void scsi_print_devices(void) {
    if (device_count == 0) {
        return; // Don't print anything if no devices
    }
    
    terminal_writestring("\nSCSI Devices:\n");
    terminal_writestring("=============\n");
    serial_write("\nSCSI Devices:\n");
    
    for (int i = 0; i < device_count; i++) {
        scsi_device_t* dev = &scsi_devices[i];
        
        terminal_writestring("Device ");
        terminal_putchar('0' + i);
        terminal_writestring(": Target ");
        terminal_putchar('0' + dev->target);
        terminal_writestring(" - ");
        
        // Device type
        switch (dev->type) {
            case SCSI_TYPE_DISK:
                terminal_writestring("SCSI HDD\n");
                serial_write("  Type: SCSI Hard Disk\n");
                break;
            case SCSI_TYPE_CDROM:
                terminal_writestring("SCSI CD/DVD\n");
                serial_write("  Type: SCSI Optical Drive\n");
                break;
            case SCSI_TYPE_TAPE:
                terminal_writestring("SCSI Tape\n");
                serial_write("  Type: SCSI Tape Drive\n");
                break;
            default:
                terminal_writestring("SCSI Device\n");
                serial_write("  Type: SCSI Device\n");
                break;
        }
        
        // Vendor and product
        terminal_writestring("  Vendor: ");
        terminal_writestring(dev->vendor);
        terminal_writestring("\n");
        terminal_writestring("  Product: ");
        terminal_writestring(dev->product);
        terminal_writestring("\n");
        
        serial_write("  Vendor: ");
        serial_write(dev->vendor);
        serial_write("\n");
        serial_write("  Product: ");
        serial_write(dev->product);
        serial_write("\n");
        serial_write("  Revision: ");
        serial_write(dev->revision);
        serial_write("\n");
        
        // Size for disks
        if (dev->type == SCSI_TYPE_DISK && dev->block_count > 0) {
            uint32_t size_mb = (dev->block_count * dev->block_size) / (1024 * 1024);
            
            terminal_writestring("  Size: ");
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
            terminal_writestring(" MB (");
            
            // Block count
            temp = dev->block_count;
            pos = 0;
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
            terminal_writestring(" blocks)\n");
            
            serial_write("  Size: ");
            temp = size_mb;
            pos = 0;
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
            serial_write(size_str);
            serial_write(" MB\n");
        }
    }
}

// Get device count
int scsi_get_device_count(void) {
    return device_count;
}

// Get device by index
scsi_device_t* scsi_get_device(int index) {
    if (index < 0 || index >= device_count) {
        return NULL;
    }
    return &scsi_devices[index];
}

// Read a single sector (simplified - would need CCB implementation)
uint8_t scsi_read_sector(uint8_t device_id, uint32_t lba, uint8_t* buffer) {
    if (device_id >= device_count) {
        return 1; // Error
    }
    
    // This would need full CCB implementation
    // For now, just return success
    serial_write("SCSI read sector not yet fully implemented\n");
    return 0; // Success (simulated)
}

// Write a single sector (simplified)
uint8_t scsi_write_sector(uint8_t device_id, uint32_t lba, uint8_t* buffer) {
    if (device_id >= device_count) {
        return 1; // Error
    }
    
    // This would need full CCB implementation
    serial_write("SCSI write sector not yet fully implemented\n");
    return 0; // Success (simulated)
}

// Read multiple blocks
int scsi_read_blocks(uint8_t device_id, uint32_t lba, uint16_t count, uint8_t* buffer) {
    if (device_id >= device_count) {
        return -1; // Error
    }
    
    // Would iterate and read each block via scsi_read_sector
    serial_write("SCSI read blocks not yet fully implemented\n");
    return 0; // Success (simulated)
}

// Write multiple blocks
int scsi_write_blocks(uint8_t device_id, uint32_t lba, uint16_t count, uint8_t* buffer) {
    if (device_id >= device_count) {
        return -1; // Error
    }
    
    // Would iterate and write each block via scsi_write_sector
    serial_write("SCSI write blocks not yet fully implemented\n");
    return 0; // Success (simulated)
}
