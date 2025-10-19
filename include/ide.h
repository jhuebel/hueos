#ifndef IDE_H
#define IDE_H

#include "kernel.h"

// IDE Controller ports
#define ATA_PRIMARY_IO       0x1F0
#define ATA_PRIMARY_CTRL     0x3F6
#define ATA_SECONDARY_IO     0x170
#define ATA_SECONDARY_CTRL   0x376

// IDE Registers
#define ATA_REG_DATA         0x00
#define ATA_REG_ERROR        0x01
#define ATA_REG_FEATURES     0x01
#define ATA_REG_SECCOUNT0    0x02
#define ATA_REG_LBA0         0x03
#define ATA_REG_LBA1         0x04
#define ATA_REG_LBA2         0x05
#define ATA_REG_HDDEVSEL     0x06
#define ATA_REG_COMMAND      0x07
#define ATA_REG_STATUS       0x07
#define ATA_REG_SECCOUNT1    0x08
#define ATA_REG_LBA3         0x09
#define ATA_REG_LBA4         0x0A
#define ATA_REG_LBA5         0x0B
#define ATA_REG_CONTROL      0x0C
#define ATA_REG_ALTSTATUS    0x0C

// Status register bits
#define ATA_SR_BSY           0x80
#define ATA_SR_DRDY          0x40
#define ATA_SR_DF            0x20
#define ATA_SR_DSC           0x10
#define ATA_SR_DRQ           0x08
#define ATA_SR_CORR          0x04
#define ATA_SR_IDX           0x02
#define ATA_SR_ERR           0x01

// Error register bits
#define ATA_ER_BBK           0x80
#define ATA_ER_UNC           0x40
#define ATA_ER_MC            0x20
#define ATA_ER_IDNF          0x10
#define ATA_ER_MCR           0x08
#define ATA_ER_ABRT          0x04
#define ATA_ER_TK0NF         0x02
#define ATA_ER_AMNF          0x01

// Commands
#define ATA_CMD_READ_PIO     0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_WRITE_PIO    0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_CACHE_FLUSH  0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET       0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY     0xEC

// ATAPI Commands
#define ATAPI_CMD_READ       0xA8
#define ATAPI_CMD_EJECT      0x1B

// Device types
#define IDE_ATA              0x00
#define IDE_ATAPI            0x01

// Device structure
typedef struct {
    uint8_t  reserved;
    uint8_t  channel;      // 0 (Primary) or 1 (Secondary)
    uint8_t  drive;        // 0 (Master) or 1 (Slave)
    uint8_t  type;         // 0: ATA, 1: ATAPI
    uint16_t signature;    // Drive signature
    uint16_t capabilities; // Features
    uint32_t command_sets; // Supported command sets
    uint32_t size;         // Size in sectors
    char     model[41];    // Model string
} ide_device_t;

// Function prototypes
void init_ide(void);
void ide_detect_devices(void);
uint8_t ide_read_sector(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t* buffer);
uint8_t ide_write_sector(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t* buffer);
void ide_print_devices(void);
int ide_get_device_count(void);
ide_device_t* ide_get_device(int index);

#endif