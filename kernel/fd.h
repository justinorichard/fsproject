#pragma once

#include "fs.h"

/**
 * @brief Create a fd object
 * 
 * @param file_entry file entry of the file
 * @return int 
 */
int create_fd(file_entry_t* file_entry);

/**
 * @brief find file entry given file descriptor
 * 
 * @param fd file descriptor
 * @return file_entry_t* pointer to the file entry
 */
file_entry_t* find_fd(int fd);

/**
 * @brief remove file descriptor object
 * 
 * @param fd file descriptor to remove
 */
void remove_fd(int fd);
