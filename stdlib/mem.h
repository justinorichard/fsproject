#include <stdint.h>
#include <stddef.h>

void *memset(void *str, int c, size_t n);

void *memcpy(void *dest, const void *src, size_t n);

void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, uint64_t offset);

void *malloc(size_t sz);

long exec(const char *file_name);