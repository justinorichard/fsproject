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

;

void _start()
{

    //   for (;;) {
    //     // init the input buffer
    //     char input_buffer[512];
    //     int next = 0;
    //     memset(input_buffer, 0, 512);

    //     // input prompt
    //     printf("$ ");

    //     char ch;
    //     while (read(1, &ch, 1)) {
    //         input_buffer[next] = ch;
    //         printf("%c", ch);
    //         // user done with input
    //         if (ch == '\n') {
    //             input_buffer[next] = '\0';
    //             if(exec(input_buffer) != 0) {
    //                 printf("%s not found\n", input_buffer);
    //                 break;
    //             }
    //         }

    //         next += 1;
    //     }
    //     // exec(buff, NULL)

    // }
  // printf("Hello User!\n");

char *buffer = malloc(BUFFER_SIZE);
//   // char *pos[NUM_MODULES] = {"hello_world",
//   //                           "init",
//   //                           "test_mod"};

  char *cmds[2] = {"file_read", "file_append"
  };
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
      syscall(SYS_EXEC, buffer);
      i = 0;
      for(int j = 0; j < 2; j++){
        if(strcmp(cmds[j], buffer) == 0){
          if(j == 0){
            file_read(0);
          }
          else{
            file_append(0, "a", 1);
          }
        }
      }
    }
    else
    {
      i++;
    }
  }
}

      // See if it is one of the possible commands.
      // int exists = 0;
      // for (int j = 0; j < 2; j++)
      // {
      //   if (strcmp(cmds[j], buffer) == 0)
      //   {
      //     exists = 1;
      //     break;
      //   }
      // }
      // // if not a command
      // if (exists == 0)
      // {
      //   // clear the buffer and print that the command does not exist
      //   printf("invalid command\n");
      // }
      // else if (exists == 1)
      // {
      //   // Call exec on only the module
      //   syscall(SYS_EXEC, buffer);
      // }
  //   }
  //   else
  //   {
  //     i++;
  //   }
  // }