#pragma once

#include "fs.h"

int create_fd(file_entry_t* file_entry);

file_entry_t* find_fd(int fd);

void remove_fd(int fd);
