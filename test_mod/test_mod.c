#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mem.h>
#include <io.h>

#include "syscall.h"


void _start()
{
  printf("Open f1.txt\n");
  int fd1 = file_open("f1.txt");
  printf("Open f2.txt\n");
  int fd2 = file_open("f2.txt");

  printf("write 'hello' to f1.txt\n");
  if (!file_write(fd1, "hello", 5)) {
    printf("write failed!\n");
  }

  printf("write 'nihao' to f2.txt\n");
  if (!file_write(fd2, "nihao", 5)) {
    printf("write failed!\n");
  }

  printf("Currently Stored in f1.txt: \n");
  file_read(fd1);
  printf("\nCurrently Stored in f2.txt: \n");
  file_read(fd2);

  printf("\n\nappend ' world' to f1.txt\n");
  file_append(fd1, " world\n", 7);
  printf("set first two letters to 'a ' in f2.txt\n");
  file_write_at(fd2, 0, "a ", 2);

  printf("\nCurrently Stored in f1.txt: \n");
  file_read(fd1);
  printf("\nCurrently Stored in f2.txt: \n");
  file_read(fd2);

  printf("\n\nDelete f1.txt\n");
  file_delete(fd1);

  printf("Try to read file f1.txt\n");
  file_read(fd1);

  printf("\nTry to write to f1.txt\n");
  file_write(fd1);

  syscall(SYS_EXIT);
}