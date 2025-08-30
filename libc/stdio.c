/**
 * Maya OS Standard I/O Library
 * Updated: 2025-08-29 10:59:28 UTC
 * Author: AmanNagtodeOfficial
 */

#include "libc/stdio.h"
#include "drivers/vga.h"
#include "drivers/serial.h"
#include "kernel/memory.h"
#include <stdarg.h>

#define PRINTF_BUFFER_SIZE 4096
#define MAX_NUMBER_LENGTH 32

static char printf_buffer[PRINTF_BUFFER_SIZE];
static bool stdio_initialized = false;

bool stdio_init(void) {
    if (stdio_initialized) {
        return true;
    }

    if (!vga_init() || !serial_init(COM1)) {
        return false;
    }

    stdio_initialized = true;
    return true;
}

int putchar(int c) {
    if (!stdio_initialized) {
        return EOF;
    }

    char ch = (char)c;
    vga_putchar(ch);
    serial_write(COM1, &ch, 1);
    return c;
}

int puts(const char* str) {
    if (!stdio_initialized || !str) {
        return EOF;
    }

    while (*str) {
        if (putchar(*str++) == EOF) {
            return EOF;
        }
    }
    
    putchar('\n');
    return 1;
}

static void print_number(unsigned long num, int base, bool is_signed, bool uppercase) {
    char buffer[MAX_NUMBER_LENGTH];
    char* ptr = &buffer[MAX_NUMBER_LENGTH - 1];
    *ptr = '\0';

    if (is_signed && (long)num < 0) {
        num = -(long)num;
        putchar('-');
    }

    if (num == 0) {
        putchar('0');
        return;
    }

    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    while (num > 0) {
        *--ptr = digits[num % base];
        num /= base;
    }

    while (*ptr) {
        putchar(*ptr++);
    }
}

int printf(const char* format, ...) {
    if (!stdio_initialized || !format) {
        return -1;
    }

    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}

int vprintf(const char* format, va_list args) {
    if (!stdio_initialized || !format) {
        return -1;
    }

    int written = 0;
    bool in_format = false;
    bool long_arg = false;
    bool uppercase = false;
    bool zero_pad = false;
    int min_width = 0;

    while (*format) {
        if (in_format) {
            switch (*format) {
                case 'l':
                    long_arg = true;
                    break;

                case '0':
                    if (min_width == 0) {
                        zero_pad = true;
                        break;
                    }
                    // Fall through if not at start of width

                case '1'...'9':
                    min_width = min_width * 10 + (*format - '0');
                    break;

                case 'd':
                case 'i':
                    if (long_arg) {
                        print_number(va_arg(args, long), 10, true, false);
                    } else {
                        print_number(va_arg(args, int), 10, true, false);
                    }
                    written++;
                    in_format = false;
                    break;

                case 'u':
                    if (long_arg) {
                        print_number(va_arg(args, unsigned long), 10, false, false);
                    } else {
                        print_number(va_arg(args, unsigned int), 10, false, false);
                    }
                    written++;
                    in_format = false;
                    break;

                case 'x':
                case 'X':
                    uppercase = (*format == 'X');
                    if (long_arg) {
                        print_number(va_arg(args, unsigned long), 16, false, uppercase);
                    } else {
                        print_number(va_arg(args, unsigned int), 16, false, uppercase);
                    }
                    written++;
                    in_format = false;
                    break;

                case 'c':
                    putchar(va_arg(args, int));
                    written++;
                    in_format = false;
                    break;

                case 's': {
                    const char* str = va_arg(args, const char*);
                    if (!str) str = "(null)";
                    while (*str) {
                        putchar(*str++);
                        written++;
                    }
                    in_format = false;
                    break;
                }

                case 'p':
                    putchar('0');
                    putchar('x');
                    print_number(va_arg(args, unsigned long), 16, false, false);
                    written++;
                    in_format = false;
                    break;

                case '%':
                    putchar('%');
                    written++;
                    in_format = false;
                    break;

                default:
                    putchar('%');
                    putchar(*format);
                    written += 2;
                    in_format = false;
                    break;
            }
        } else if (*format == '%') {
            in_format = true;
            long_arg = false;
            uppercase = false;
            zero_pad = false;
            min_width = 0;
        } else {
            putchar(*format);
            written++;
        }
        format++;
    }

    return written;
}

int sprintf(char* buffer, const char* format, ...) {
    if (!buffer || !format) {
        return -1;
    }

    va_list args;
    va_start(args, format);
    int result = vsprintf(buffer, format, args);
    va_end(args);
    return result;
}

int snprintf(char* buffer, size_t size, const char* format, ...) {
    if (!buffer || !format || size == 0) {
        return -1;
    }

    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}

bool stdio_is_initialized(void) {
    return stdio_initialized;
}
