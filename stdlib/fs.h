#include <stdint.h>
#include <stddef.h>

int file_open(const char* file_name);

bool file_write(int fd, uint8_t* buf, size_t size);

bool file_write_at(int fd, size_t index, uint8_t* buff, size_t size);

bool file_append(int fd, uint8_t* buf, size_t size);

void file_read(int fd);

void file_delete(int fd);

bool file_rename(const char* oldpath, const char* newpath);



