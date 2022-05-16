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
#define MAX_BLOCK_SIZE 10
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


void fs_init();

int fs_open(const char* file_name);

bool fs_write(int fd, uint8_t* buf, size_t size);

bool fs_write_at(int fd, size_t index, uint8_t* buff, size_t size);

bool fs_append(int fd, uint8_t* buf, size_t size);

void fs_read(int fd);

void fs_delete(int fd);

bool fs_rename(const char* oldpath, const char* newpath);
