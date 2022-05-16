#include <stdint.h>
#include <stddef.h>

void *file_append(int fd, uint8_t* buf, size_t size);

void *file_read(int fd);

