#include "fs.h"

#include <string.h>

#include "fd.h"


data_block_t *blocks = NULL;
node_t *free_list = NULL;

file_entry_t entries[MAX_FILE_SIZE];
size_t next_entry = 0;

void fs_init() {
    blocks = (data_block_t*)START_ADDR;

    // adding data blocks to the free list
    for (size_t i = 0; i < MAX_BLOCK_SIZE+1; i++) {
        uintptr_t curr_block = START_ADDR + i * 0x1000;
        vm_map(read_cr3() & ~0xFFFLL, curr_block, 1, 1, 1);
        node_t* curr_node = (node_t*)curr_block;
        curr_node->next = free_list;
        free_list = curr_node;
    }
}

int fs_open(const char *file_name) {
    if (next_entry >= MAX_FILE_SIZE) {
        return -1;
    }

    // Find file and set fd
    for (size_t i = 0; i < next_entry; i++) {
        if (strcmp(file_name, entries[i].name) == 0) {
            int fd = create_fd(&entries[i]);
            return fd;
        }
    }

    // file not found, create a new file instead
    // sanity check
    if (next_entry >= MAX_FILE_SIZE) {
        return -1;
    }
    if (free_list == NULL) {
        return -1;
    }

    // setting up an empty file entry
    strcpy(entries[next_entry].name, file_name);
    entries[next_entry].file_size = 0;
    entries[next_entry].start_index = ((uintptr_t)free_list - (uintptr_t)blocks) / BLOCK_SIZE;
    data_block_t *new_block = (data_block_t *)free_list;
    free_list = free_list->next;
    new_block->index = BLOCK_INDEX_END;

    int fd = create_fd(&entries[next_entry]);
    next_entry += 1;

    return fd;
}

size_t min(size_t num1, size_t num2) { return num1 > num2 ? num2 : num1; }

size_t max(size_t num1, size_t num2) { return num1 > num2 ? num1 : num2; }

bool fs_write(int fd, uint8_t *buf, size_t size) { return fs_write_at(fd, 0, buf, size); }

bool fs_write_at(int fd, size_t index, uint8_t *buff, size_t size) {
    file_entry_t *entry = find_fd(fd);
    if(entry == NULL)
        return false;
    size_t fsize = entry->file_size;

    if (index > fsize) {
        // writing outside the bound
        return false;
    }

    int num_blocks = max(1, (fsize + BLOCK_DATA_SIZE - 1) / BLOCK_DATA_SIZE);
    int blocks_needed = (index + size + BLOCK_DATA_SIZE - 1) / BLOCK_DATA_SIZE;

    if (blocks_needed > num_blocks) {  // need allocate new block
        // go to the end of data block of file
        data_block_t *curr_block = &blocks[entry->start_index];
        while (curr_block->index != BLOCK_INDEX_END) {
            curr_block = &blocks[curr_block->index];
        }

        // allocate new block
        int num_new_block = blocks_needed - num_blocks;
        for (size_t i = 0; i < num_new_block; i++) {
            if (free_list == NULL) {
                return false;
            }

            data_block_t *new_block = (data_block_t *)free_list;
            free_list = free_list->next;

            curr_block->index = ((uintptr_t)new_block - (uintptr_t)blocks) / BLOCK_SIZE;
            curr_block = new_block;

            // set index to last block
            if (i == (num_new_block - 1)) {
                curr_block->index = BLOCK_INDEX_END;
            }
        }
    }

    // Skip number of blocks to get to index to start writing at.
    size_t start_block = index / BLOCK_DATA_SIZE;
    size_t write_point = index % BLOCK_DATA_SIZE;

    // Loop through each block to get to desired
    data_block_t *curr_block = &blocks[entry->start_index];
    for (int i = 0; i < start_block; i++) {  // EOF
        curr_block = &blocks[curr_block->index];
    }

    entry->file_size = max(index + size, entry->file_size);
    // Once we are at block, start writing byte by byte
    while (size > 0) {
        // change write point if we reach the end
        if (write_point == BLOCK_DATA_SIZE) {
            curr_block = &blocks[curr_block->index];
            write_point = 0;
        }
        memcpy((curr_block->data) + write_point++, buff++, 1);
        size--;
    }

    return true;
}

bool fs_append(int fd, uint8_t *buf, size_t size) {
    file_entry_t *entry = find_fd(fd);
    if(entry == NULL)
        return false;
    return fs_write_at(fd, entry->file_size, buf, size);
}

void fs_read(int fd) {
    file_entry_t *entry = find_fd(fd);
    if(entry == NULL){
        kprintf("WARNING: The file you are trying to read does not exist\n");
        return;
    }
    size_t total_size = entry->file_size;
    data_block_t *curr_block = &blocks[entry->start_index];
    for (;;) {
        for (size_t j = 0; j < min(BLOCK_DATA_SIZE, total_size); j++) {
            kprintf("%c", curr_block->data[j]);
        }
        total_size -= BLOCK_DATA_SIZE;

        // move to next block
        if (curr_block->index == BLOCK_INDEX_END) {  // EOF
            break;
        }
        curr_block = &blocks[curr_block->index];
    }
}

void fs_delete(int fd) {
    file_entry_t *entry = find_fd(fd);
    if(entry == NULL){
        kprintf("WARNING: The file you are trying to delete does not exist\n");
        return;
    }
    // first block
    data_block_t *curr_block = &blocks[entry->start_index];
    node_t *curr_node;
    while (curr_block->index != BLOCK_INDEX_END) {  // EOF
        // cast to node
        node_t *curr_node = (node_t *)curr_block;
        curr_node->next = free_list;
        free_list = curr_node;
        curr_block = &blocks[curr_block->index];
    }
    // Remove entry
    strcpy(entry->name, "");
    memset(&entry->start_index, 0, sizeof(uint32_t));
    memset(&entry->file_size, 0, sizeof(size_t));
    remove_fd(fd);
}

bool fs_rename(const char *oldpath, const char *newpath) {
    for (int i = 0; i < next_entry; i++) {
        if (strcmp(oldpath, entries[i].name) == 0) {
            strcpy(entries[i].name, newpath);
            return true;
        }
    }

    return false;
}
