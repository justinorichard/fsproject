#include <stdint.h>
#include <stddef.h>
#include <io.h>
#include <stdbool.h>

#include "fs.h"

#define SYS_FS_OPEN 12
#define SYS_FS_WRITE 13
#define SYS_FS_WRITE_AT 14
#define SYS_FS_APPEND 15
#define SYS_FS_READ 16
#define SYS_FS_DELETE 17 
#define SYS_FS_RENAME 18

int file_open(const char* file_name){
    return syscall(SYS_FS_OPEN, file_name);
}

bool file_write(int fd, uint8_t* buf, size_t size){
    return syscall(SYS_FS_WRITE, fd, buf, size);
}

bool file_write_at(int fd, size_t index, uint8_t* buff, size_t size){
    return syscall(SYS_FS_WRITE_AT, index, buff, size);
}

bool file_append(int fd, uint8_t* buf, size_t size){
    return syscall(SYS_FS_APPEND, fd, buf, size);
}

void file_read(int fd){
    syscall(SYS_FS_READ, fd);
}

void file_delete(int fd){
    syscall(SYS_FS_DELETE, fd);
}

bool file_rename(const char* oldpath, const char* newpath){
    syscall(SYS_FS_RENAME, oldpath, newpath);
}

