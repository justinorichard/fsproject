#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define START_ADDR 0xffff80007fc42000

/*
0xffff80000003d000-0xffff80000009f000
0xffff800000100000-0xffff80007ede4000
0xffff80007fc42000-0xffff80007ff62000
*/

// data block definition (in bytes)
#define BLOCK_INDEX_SIZE 4
#define BLOCK_DATA_SIZE 124
#define BLOCK_SIZE (BLOCK_INDEX_SIZE + BLOCK_DATA_SIZE)
#define MAX_BLOCK_SIZE 128
#define BLOCK_INDEX_END UINT32_MAX

// file entry
#define MAX_FILENAME 32
#define MAX_FILE_SIZE 256

typedef struct file_entry {
    char name[MAX_FILENAME];
    uint32_t start_index;
    size_t file_size;
} file_entry_t;


typedef struct __attribute__((__packed__)) data_block {
    uint32_t index;
    uint8_t data[BLOCK_DATA_SIZE];
} data_block_t;

typedef struct node {
    struct node *next;
} node_t;

/**
 * @brief Initialize data blocks for file system
 */
void fs_init();

/**
 * @brief Open a file with the given name
 * 
 * @param file_name name of the file
 * @return file descriptor integer
 */
int fs_open(const char* file_name);

/**
 * @brief Write to a file
 * 
 * @param fd file descriptor of the file to write to
 * @param buf array of characters to write to the file
 * @param size size of the input buffer
 * @return true when write success
 * @return false when write fails
 */
bool fs_write(int fd, uint8_t* buf, size_t size);

/**
 * @brief write to a file at the given index
 * 
 * @param fd file descriptor of the file to write to
 * @param index index in the file to start writing to
 * @param buff array of characters to write to the file
 * @param size size of the input buffer
 * @return true when write_at success
 * @return false when write_at fails
 */
bool fs_write_at(int fd, size_t index, uint8_t* buff, size_t size);

/**
 * @brief append to a file
 * 
 * @param fd file descriptor of the file to write to
 * @param buf array of characters to write to the file
 * @param size size of the input buffer
 * @return true when append sucess
 * @return false append fails
 */
bool fs_append(int fd, uint8_t* buf, size_t size);

/**
 * @brief read from a file
 * 
 * @param fd file descriptor of the file to write to
 */
void fs_read(int fd);

/**
 * @brief delete a file
 * 
 * @param fd file descriptor of the file to write to
 */
void fs_delete(int fd);

/**
 * @brief rename a file
 * 
 * @param oldpath old name of the file
 * @param newpath new name of the file
 * @return true when rename succeeds
 * @return false when rename fails
 */
bool fs_rename(const char* oldpath, const char* newpath);
