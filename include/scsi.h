#ifndef SCSI_H
#define SCSI_H

#include "kernel.h"

// SCSI Device Types
#define SCSI_TYPE_DISK          0x00
#define SCSI_TYPE_TAPE          0x01
#define SCSI_TYPE_PRINTER       0x02
#define SCSI_TYPE_PROCESSOR     0x03
#define SCSI_TYPE_WORM          0x04
#define SCSI_TYPE_CDROM         0x05
#define SCSI_TYPE_SCANNER       0x06
#define SCSI_TYPE_OPTICAL       0x07
#define SCSI_TYPE_MEDIUM_CHANGER 0x08
#define SCSI_TYPE_COMM          0x09
#define SCSI_TYPE_RAID          0x0C
#define SCSI_TYPE_ENCLOSURE     0x0D
#define SCSI_TYPE_RBC           0x0E
#define SCSI_TYPE_NO_DEVICE     0x7F

// SCSI Commands
#define SCSI_CMD_TEST_UNIT_READY    0x00
#define SCSI_CMD_REQUEST_SENSE      0x03
#define SCSI_CMD_INQUIRY            0x12
#define SCSI_CMD_MODE_SENSE_6       0x1A
#define SCSI_CMD_START_STOP_UNIT    0x1B
#define SCSI_CMD_READ_CAPACITY_10   0x25
#define SCSI_CMD_READ_10            0x28
#define SCSI_CMD_WRITE_10           0x2A
#define SCSI_CMD_VERIFY_10          0x2F
#define SCSI_CMD_READ_16            0x88
#define SCSI_CMD_WRITE_16           0x8A

// BusLogic PCI IDs
#define BUSLOGIC_VENDOR_ID          0x104B
#define BUSLOGIC_DEVICE_ID          0x1040

// LSI Logic PCI IDs
#define LSI_VENDOR_ID               0x1000
#define LSI_53C895A_DEVICE_ID       0x0012
#define LSI_53C1030_DEVICE_ID       0x0030

// Controller Types
#define SCSI_CONTROLLER_BUSLOGIC    0x01
#define SCSI_CONTROLLER_LSI_LOGIC   0x02

// BusLogic Mailbox Commands
#define BUSLOGIC_MBOX_CMD_FREE      0x00
#define BUSLOGIC_MBOX_CMD_START     0x01
#define BUSLOGIC_MBOX_CMD_ABORT     0x02

// BusLogic Status Codes
#define BUSLOGIC_STATUS_FREE        0x00
#define BUSLOGIC_STATUS_SUCCESS     0x01
#define BUSLOGIC_STATUS_ABORTED     0x02
#define BUSLOGIC_STATUS_NOT_FOUND   0x03
#define BUSLOGIC_STATUS_INVALID     0x04

// BusLogic Registers (I/O Port Offsets)
#define BUSLOGIC_REG_CONTROL        0x00
#define BUSLOGIC_REG_STATUS         0x00
#define BUSLOGIC_REG_COMMAND        0x01
#define BUSLOGIC_REG_DATA_IN        0x01
#define BUSLOGIC_REG_INTERRUPT      0x02
#define BUSLOGIC_REG_GEOMETRY       0x03

// Status Register Bits
#define BUSLOGIC_STATUS_CMD_INVALID 0x01
#define BUSLOGIC_STATUS_RESERVED    0x02
#define BUSLOGIC_STATUS_CMD_PARAM   0x04
#define BUSLOGIC_STATUS_HOST_READY  0x08
#define BUSLOGIC_STATUS_INIT_REQ    0x10
#define BUSLOGIC_STATUS_DIAG_FAIL   0x20
#define BUSLOGIC_STATUS_DIAG_ACTIVE 0x40
#define BUSLOGIC_STATUS_HARD_RESET  0x80

// Control Register Commands
#define BUSLOGIC_CTRL_HARD_RESET    0x40
#define BUSLOGIC_CTRL_SOFT_RESET    0x80

// Host Adapter Commands
#define BUSLOGIC_CMD_INQUIRY        0x04
#define BUSLOGIC_CMD_INITIALIZE_MBX 0x01
#define BUSLOGIC_CMD_START_SCSI     0x02
#define BUSLOGIC_CMD_EXECUTE_SCSI   0x81

// Maximum devices
#define SCSI_MAX_DEVICES            16
#define SCSI_MAX_CONTROLLERS        4

// SCSI Device Structure
typedef struct {
    uint8_t controller_id;      // Which controller this device is on
    uint8_t target;             // SCSI target ID (0-15)
    uint8_t lun;                // Logical Unit Number
    uint8_t type;               // Device type (disk, cdrom, etc.)
    uint32_t block_count;       // Total number of blocks
    uint32_t block_size;        // Size of each block in bytes
    char vendor[9];             // Vendor ID (8 chars + null)
    char product[17];           // Product ID (16 chars + null)
    char revision[5];           // Revision (4 chars + null)
} scsi_device_t;

// SCSI Controller Structure
typedef struct {
    uint8_t type;               // Controller type (BusLogic, LSI, etc.)
    uint16_t io_base;           // Base I/O port
    uint32_t mmio_base;         // Memory-mapped I/O base (if used)
    uint8_t irq;                // IRQ number
    uint8_t device_count;       // Number of devices on this controller
} scsi_controller_t;

// Command Control Block (CCB) for BusLogic
typedef struct __attribute__((packed)) {
    uint8_t opcode;             // Operation code
    uint8_t address_control;    // Address and control
    uint8_t cdb_length;         // CDB length
    uint8_t sense_length;       // Request sense length
    uint32_t data_length;       // Data transfer length
    uint32_t data_pointer;      // Data buffer pointer
    uint32_t reserved1;
    uint8_t host_status;        // Host adapter status
    uint8_t target_status;      // Target device status
    uint8_t target_id;          // Target ID
    uint8_t lun;                // Logical unit
    uint8_t cdb[12];            // Command Descriptor Block
    uint8_t reserved2[6];
    uint32_t sense_pointer;     // Sense data pointer
} buslogic_ccb_t;

// SCSI Inquiry Response
typedef struct __attribute__((packed)) {
    uint8_t peripheral_type;    // Device type
    uint8_t rmb;                // Removable media bit
    uint8_t version;            // SCSI version
    uint8_t response_format;    // Response data format
    uint8_t additional_length;  // Additional length
    uint8_t flags1;
    uint8_t flags2;
    uint8_t flags3;
    char vendor[8];             // Vendor identification
    char product[16];           // Product identification
    char revision[4];           // Product revision
} scsi_inquiry_t;

// SCSI Read Capacity Response
typedef struct __attribute__((packed)) {
    uint32_t last_lba;          // Last logical block address
    uint32_t block_size;        // Block size in bytes
} scsi_capacity_t;

// Function prototypes
void init_scsi(void);
void scsi_scan_devices(void);
void scsi_print_devices(void);
int scsi_get_device_count(void);
scsi_device_t* scsi_get_device(int index);
uint8_t scsi_read_sector(uint8_t device_id, uint32_t lba, uint8_t* buffer);
uint8_t scsi_write_sector(uint8_t device_id, uint32_t lba, uint8_t* buffer);
int scsi_read_blocks(uint8_t device_id, uint32_t lba, uint16_t count, uint8_t* buffer);
int scsi_write_blocks(uint8_t device_id, uint32_t lba, uint16_t count, uint8_t* buffer);

#endif
