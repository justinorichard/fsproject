#include <stdint.h>
#include <stddef.h>
#include <io.h>
#include <stdbool.h>

#define SYS_FS_APPEND 12
#define SYS_FS_READ 13

void *file_append(int fd, uint8_t* buf, size_t size){
    return (void*)syscall(SYS_FS_APPEND, fd, buf, size);
}

void *file_read(int fd){
    return (void*)syscall(SYS_FS_READ, fd);
}