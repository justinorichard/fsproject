#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define STD_IN 0
#define STD_OUT 1
#define STD_ERROR 2
#define SYS_READ 0
#define SYS_WRITE 1

extern int syscall(uint64_t nr, ...);

// User side read. Issues syscall to read from keyboard.
size_t read(int fd, void *buf, size_t count)
{
    return syscall(SYS_READ, fd, buf, count);
}

// User side write. Issues syscall to write to terminal.
size_t write(int fd, const void *buf, size_t count)
{
    return syscall(SYS_WRITE, fd, buf, count);
}

/**
 * Print a single char to the terminal
 * \param c char to print
 */
void print_c(char c)
{
    write(STD_OUT, &c, 1);
}

/**
 * Print a string to the terminal
 * \param str string to print
 */
void print_s(char *str)
{
    while (*str)
    {
        write(STD_OUT, str, 1);
        str++;
    }
}

/**
 * Print an unsigned 64-bit integer value to the terminal in decimal notation
 * \param value integer value to print
 */
void print_d(uint64_t value)
{
    int UINT64_LENGTH = 20;
    char str[UINT64_LENGTH];
    int i = UINT64_LENGTH;
    do
    {
        str[--i] = ((value % 10) + '0');
        value /= 10;
    } while (value != 0);
    write(STD_OUT, str + i, UINT64_LENGTH - i);
}

/**
 * Print an unsigned 64-bit integer value to the terminal in lowercase hexadecimal notation
 * \param value integer value to print
 */
void print_x(uint64_t value)
{
    int HEX64_LENGTH = 20;
    char str[HEX64_LENGTH];
    int i = HEX64_LENGTH;
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
    write(STD_OUT, str + i, HEX64_LENGTH - i);
}

/**
 * rint the value of a pointer to the terminal in lowercase hexadecimal with the prefix “0x”
 * \param ptr pointer to print
 */
void print_p(void *ptr)
{
    print_s("0x");
    print_x((uint64_t)ptr);
}

/**
 * Print a string to the terminal with format specifiers
 * \param c strings to print
 */
void printf(const char *format, ...)
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
                print_c(*pos);
                break;
            case 'c':
                print_c(va_arg(args, int));
                break;
            case 's':
                print_s(va_arg(args, char *));
                break;
            case 'd':
                print_d(va_arg(args, uint64_t));
                break;
            case 'x':
                print_x(va_arg(args, uint64_t));
                break;
            case 'p':
                print_p(va_arg(args, void *));
                break;
            default:
                print_s("????");
            }
        }
        else
        {
            print_c(*pos);
        }

        pos++;
    }

    va_end(args);
}