/* io.c - I/O utility functions and serial communication */
#include "io.h"

#define COM1 0x3F8   /* I/O port base address for COM1 */

// Serial port driver
void serial_init(void) {
    outb(COM1 + 1, 0x00);    /* Disable interrupts */
    outb(COM1 + 3, 0x80);    /* Enable DLAB (set baud rate divisor) */
    outb(COM1 + 0, 0x03);    /* Divisor low byte (38400 baud) */
    outb(COM1 + 1, 0x00);    /* Divisor high byte */
    outb(COM1 + 3, 0x03);    /* 8 bits, no parity, 1 stop bit */
    outb(COM1 + 2, 0xC7);    /* Enable FIFO, clear, 14-byte threshold */
    outb(COM1 + 4, 0x0B);    /* IRQs enabled, RTS/DSR set */
}

static int is_transmit_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_putc(char c) {
    if (c == '\n') {
        serial_putc('\r');  /* Add carriage return */
    }
    while (!is_transmit_empty());
    outb(COM1, c);
}

void serial_puts(const char* str) {
    while (*str) {
        serial_putc(*str++);
    }
}

static int serial_received(void) {
    return inb(COM1 + 5) & 0x01;
}

char serial_getc(void) {
    while (!serial_received());
    return inb(COM1);
}

// Simple itoa function for integers
static void itoa(int num, char* str, int base) {
    int i = 0;
    int isNegative = 0;
    
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    
    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }
    
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
    
    if (isNegative)
        str[i++] = '-';
    
    str[i] = '\0';
    
    // Reverse string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Simple printf for serial output (supports %d, %u, %x, %s, %c)
void printf_serial(const char* format, ...) {
    char** arg = (char**)&format;
    arg++;  // Point to first argument after format
    
    char c;
    char buf[20];
    
    while ((c = *format++)) {
        if (c != '%') {
            serial_putc(c);
        } else {
            c = *format++;
            switch (c) {
                case 'd':
                case 'i': {
                    int val = *((int*)arg);
                    arg++;
                    itoa(val, buf, 10);
                    serial_puts(buf);
                    break;
                }
                case 'u': {
                    unsigned int val = *((unsigned int*)arg);
                    arg++;
                    itoa(val, buf, 10);
                    serial_puts(buf);
                    break;
                }
                case 'x': {
                    unsigned int val = *((unsigned int*)arg);
                    arg++;
                    itoa(val, buf, 16);
                    serial_puts(buf);
                    break;
                }
                case 's': {
                    char* str = *((char**)arg);
                    arg++;
                    serial_puts(str);
                    break;
                }
                case 'c': {
                    char ch = *((char*)arg);
                    arg++;
                    serial_putc(ch);
                    break;
                }
                case '%': {
                    serial_putc('%');
                    break;
                }
                default:
                    serial_putc('%');
                    serial_putc(c);
                    break;
            }
        }
    }
}
