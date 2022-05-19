#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mem.h>
#include <io.h>
#include <string.h>

#include "syscall.h"
#include "fs.h"

#define BUFFER_SIZE 256
#define NUM_MODULES 3

void _start()
{
  printf("Hello User!\n");

  char *buffer = malloc(BUFFER_SIZE);
  char *pos[NUM_MODULES] = {"hello_world",
                            "init",
                            "test_mod"};

  int i = 0;
  while (1)
  {
    if (i == BUFFER_SIZE)
    {
      printf("Resetting Buffer\n");
      i = 0;
    }
    // read from stdin
    read(0, (buffer + i), 1);
    // write what is input
    write(1, (buffer + i), 1);
    // If latest character is the newline
    if (buffer[i] == '\n')
    {
      buffer[i] = '\0';
      i = 0;

      // See if it is one of the possible commands.
      int exists = 0;
      for (int j = 0; j < NUM_MODULES; j++)
      {
        if (strcmp(pos[j], buffer) == 0)
        {
          exists = 1;
          break;
        }
      }
      // if not a command
      if (exists == 0)
      {
        // clear the buffer and print that the command does not exist
        printf("invalid command\n");
      }
      else if (exists == 1)
      {
        // Call exec on only the module
        syscall(SYS_EXEC, buffer);
      }
    }
    else
    {
      i++;
    }
  }
} 