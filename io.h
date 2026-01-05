/* io.h - I/O operations and serial communication */
#ifndef IO_H
#define IO_H

#include "types.h"

// Low-level port I/O
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Serial port functions
void serial_init(void);
void serial_putc(char c);
void serial_puts(const char* str);
char serial_getc(void);

// Printf-like function for serial output
void printf_serial(const char* format, ...);

#endif
