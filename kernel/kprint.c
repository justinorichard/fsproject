#include "kprint.h"
#include "terminal.h"

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

/**
 * Write to the terminal
 * \param str a string to write to the terminal
 * \param len The length of the string being written to terminal.
 */
void term_write(char *str, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        term_putchar(str[i]);
    }
}

/**
 * Print a single character to the terminal
 * \param c character to print
 */
void kprint_c(char c)
{
    term_write(&c, 1);
}


/**
 * Print a string to the terminal
 * \param str string to print
 */
void kprint_s(char *str)
{
    while (*str)
    {
        term_write(str, 1);
        str++;
    }
}

/**
 * Print an unsigned 64-bit integer value to the terminal in decimal notation
 * \param value value to print
 */
void kprint_d(uint64_t value)
{
    // Max number of digits
    int UINT64_LENGTH = 20;
    // String to be printed
    char str[UINT64_LENGTH];
    // Start index at end of string
    int i = UINT64_LENGTH;
    // Loop through each digit in the integer
    do
    {
        // place last digit at end of output string and move to previous
        str[--i] = ((value % 10) + '0');
        value /= 10;
    } while (value != 0);
    // write the digits to the terminal
    term_write(str + i, UINT64_LENGTH - i);
}

/**
 * Print an unsigned 64-bit integer value to the terminal in lowercase hexadecimal notation
 * \param value value to print
 */
void kprint_x(uint64_t value)
{
    // Max number of digits
    int HEX64_LENGTH = 20;
    // String to be printed
    char str[HEX64_LENGTH];
    int i = HEX64_LENGTH;
    // Loop through each digit in the integer
    do
    {
        int remainder = value % 16;
        if (remainder > 9)
        {
            remainder += 39;
        }
        str[--i] = (remainder + '0');
        value /= 16;
    } while (value != 0);
    term_write(str + i, HEX64_LENGTH - i);
}

/**
 * Print the value of a pointer to the terminal in lowercase hexadecimal with the prefix “0x”
 * \param ptr pointer to print
 */
void kprint_p(void *ptr)
{
    term_write("0x", 2);
    kprint_x((uint64_t)ptr);
}

/**
 * Print a string to the terminal with format specifiers
 * \param c strings to print
 */
void kprintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    const char *pos = format;
    while (*pos)
    {
        if (*pos == '%')
        {
            pos++;
            switch (*pos)
            {
            case '%':
                kprint_c(*pos);
                break;
            case 'c':
                kprint_c(va_arg(args, int));
                break;
            case 's':
                kprint_s(va_arg(args, char *));
                break;
            case 'd':
                kprint_d(va_arg(args, uint64_t));
                break;
            case 'x':
                kprint_x(va_arg(args, uint64_t));
                break;
            case 'p':
                kprint_p(va_arg(args, void *));
                break;
            default:
                kprint_s("????");
            }
        }
        else
        {
            kprint_c(*pos);
        }

        pos++;
    }

    va_end(args);
}