#ifndef SERIAL_H
#define SERIAL_H

#include "kernel.h"

#define COM1 0x3F8

void init_serial(void);
void serial_write(const char* data);
void serial_writechar(char c);
void serial_write_hex(uint32_t value);

#endif