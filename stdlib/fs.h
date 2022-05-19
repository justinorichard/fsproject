#include <stdint.h>
#include <stddef.h>

void *file_open(const char* file_name);

void *file_write(int fd, uint8_t* buf, size_t size);

void *file_write_at(int fd, size_t index, uint8_t* buff, size_t size);

void *file_append(int fd, uint8_t* buf, size_t size);

void *file_read(int fd);

void *file_delete(int fd);

void *file_rename(const char* oldpath, const char* newpath);



