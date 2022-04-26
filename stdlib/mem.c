#include <stdint.h>
#include <stddef.h>
#include <io.h>

#define PAGE_SIZE 0x1000
#define SYS_MMAP 9

#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_NONE 8
#define PROT_EXEC 4
#define MAP_PRIVATE 2
#define MAP_ANONYMOUS 0

// mmap
// malloc
// memcpy
// memset

// Round a value x up to the next multiple of y
#define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

extern int64_t syscall(uint64_t nr, ...);

void *memset(void *str, int c, size_t n)
{
    while (n > 0)
    {
        *(uint8_t *)str = c;
        str++;
        n--;
    }
    return str;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    for (int i = 0; i < n; i++)
    {
        ((int8_t *)dest)[i] = ((int8_t *)src)[i];
    }
    return dest;
}

void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, uint64_t offset)
{
    // If length is less than or equal to 0
    if (length <= 0)
    {
        return (void *)-1;
    }

    return (void *)syscall(SYS_MMAP, addr, length, prot, flags, fd, offset);
}

void *bump = NULL;
size_t space_remaining = 0;

void *malloc(size_t sz)
{
    // Round sz up to a multiple of 16
    sz = ROUND_UP(sz, 16);

    // Do we have enough space to satisfy this allocation?
    if (space_remaining < sz)
    {
        // No. Get some more space using `mmap`
        size_t rounded_up = ROUND_UP(sz, PAGE_SIZE);
        void *newmem = mmap(NULL, rounded_up, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        // Check for errors
        if (newmem == NULL)
        {
            return NULL;
        }

        bump = newmem;
        space_remaining = rounded_up;
    }

    // Grab bytes from the beginning of our bump pointer region
    void *result = bump;
    bump += sz;
    space_remaining -= sz;

    return result;
}

void free(void *p)
{
    // Do nothing
}