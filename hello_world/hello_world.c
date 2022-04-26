#include <stddef.h>
#include <io.h>

#include "syscall.h"

void _start()
{
  printf("Hello World\n");

  syscall(SYS_EXIT);
}