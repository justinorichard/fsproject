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
#define SYS_FS_OPEN 12
#define SYS_FS_WRITE 13
#define SYS_FS_WRITE_AT 14
#define SYS_FS_APPEND 15
#define SYS_FS_READ 16
#define SYS_FS_DELETE 17 
#define SYS_FS_RENAME 18


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

long SYS_fs_open(const char* file_name);
long SYS_fs_write(int fd, uint8_t* buf, size_t size);
long SYS_fs_write_at(int fd, size_t index, uint8_t* buff, size_t size);
long SYS_fs_append(int fd, uint8_t *buf, size_t size);
long SYS_fs_read(int fd);
long SYS_fs_delete(int fd);
long SYS_fs_rename(const char* oldpath, const char* newpath);
