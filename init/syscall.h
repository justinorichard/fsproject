#include <stdint.h>

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_MMAP 9
#define SYS_EXEC 5
#define SYS_EXIT 6

extern int syscall(uint64_t nr, ...);