#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mem.h>
#include <io.h>

#include "syscall.h"

void _start()
{
  char *str = malloc(11);
  printf("Type 10 characters to print\n");
  read(0, str, 10);
  str[10] = '\0';
  printf("%s\n", str);

  syscall(SYS_EXIT);
}