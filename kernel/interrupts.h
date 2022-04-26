#pragma once

#define IO_BUFFER_SIZE 1000
#define L_SHIFT_PRESSED 0x2A
#define R_SHIFT_PRESSED 0x36
#define BACKSPACE_PRESSED 0x0E
#define ENTER_PRESSED 0x1C
#define L_SHIFT_RELEASED 0xAA
#define R_SHIFT_RELEASED 0xB6

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_MMAP 9
#define SYS_EXEC 5
#define SYS_EXIT 6


/**
 * Initialize an interrupt descriptor table, set handlers for standard exceptions, and install
 * the IDT.
 */
void idt_setup();

/**
 * Read one character from the keyboard buffer. If the keyboard buffer is empty this function will
 * block until a key is pressed.
 *
 * \returns the next character input from the keyboard
 */
char kgetc();

extern int64_t syscall(uint64_t nr, ...);
long SYS_read(int file_descriptor, char *buffer, int length);
long SYS_write(int file_descriptor, char *buffer, int length);
long SYS_mmap(void *addr, size_t length, int prot, int flags,
              int fd, uint64_t offset);

long SYS_exec(char *module);
long SYS_exit();
