#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <mem.h>

#include "stivale2.h"
#include "kprint.h"
#include "util.h"

#define PAGESIZE 0x1000

typedef struct page_table_entry
{
    bool present : 1;
    bool writable : 1;
    bool user : 1;
    bool write_through : 1;
    bool cache_disable : 1;
    bool accessed : 1;
    bool dirty : 1;
    bool page_size : 1;
    uint8_t _unused0 : 4;
    uintptr_t address : 40;
    uint16_t _unused1 : 11;
    bool no_execute : 1;
} __attribute__((packed)) pt_entry_t;

uint64_t hhdm_addr;
struct stivale2_struct *hdr_global;

void *
find_tag(struct stivale2_struct *hdr, uint64_t id);


// Prints out the physical and virtual address of usable memory
void usable_mem(struct stivale2_struct *hdr)
{
    struct stivale2_struct_tag_memmap *memmap_tag = find_tag(hdr, STIVALE2_STRUCT_TAG_MEMMAP_ID);
    struct stivale2_struct_tag_hhdm *hhdm_tag = find_tag(hdr, STIVALE2_STRUCT_TAG_HHDM_ID);

    // Make sure we find a terminal tag
    if (memmap_tag == NULL || hhdm_tag == NULL)
    {
        kprintf("Could not find memmap or hhdm tags");
        halt();
    }

    hhdm_addr = hhdm_tag->addr;

    // Loop through each entry
    for (int i = 0; i < memmap_tag->entries; i++)
    {
        struct stivale2_mmap_entry entry = (memmap_tag->memmap[i]);
        if (entry.type != 1)
            continue;
        // Print address physical and virtual address range
        kprintf("%p-%p mapped at %p-%p\n", entry.base, entry.base + entry.length, entry.base + hhdm_addr, entry.base + entry.length + hhdm_addr);
    }
}

// Freelist Node
typedef struct Node
{
    // holds size of chunk and address of start
    uint64_t size;
    uint64_t addr;
    struct Node *next;
} Node_t;

Node_t *FREELIST = NULL;

void freelist_init(struct stivale2_struct *hdr)
{
    // initialize list of free memory.
    // Use memmap tag to get every entry. We want to return the first chunk of
    //  usable memory and indicate that it is being used.
    struct stivale2_struct_tag_memmap *memmap_tag = find_tag(hdr, STIVALE2_STRUCT_TAG_MEMMAP_ID);
    struct stivale2_struct_tag_hhdm *hhdm_tag = find_tag(hdr, STIVALE2_STRUCT_TAG_HHDM_ID);

    // Make sure we find a terminal tag
    if (memmap_tag == NULL || hhdm_tag == NULL)
    {
        kprintf("Could not find memmap or hhdm tags");
        halt();
    }

    hhdm_addr = hhdm_tag->addr;
    hdr_global = hdr;
    // Loop through each entry
    // entry is a contiguous chunk of memory.
    for (int i = 0; i < memmap_tag->entries; i++)
    {
        struct stivale2_mmap_entry entry = (memmap_tag->memmap[i]);
        // if equal to 1, it is free
        if (entry.type == 1)
        {
            Node_t *node = (Node_t *)(entry.base + hhdm_addr);
            node->size = entry.length;
            node->addr = entry.base;
            node->next = FREELIST;
            FREELIST = node;
        }
    }
    // Print freelist
    // Node_t *curr = FREELIST;
    // while (curr != NULL)
    // {
    //     kprintf("Size: %p, Address: %p\n", curr->size, curr->addr);
    //     curr = curr->next;
    // }
}

uintptr_t ptov(uintptr_t phys_add)
{
    return phys_add + hhdm_addr;
}

/**
 * Allocate a page of physical memory.
 * \returns the physical address of the allocated physical memory or 0 on error.
 */
uintptr_t pmem_alloc()
{

    uintptr_t result = -1;
    Node_t *curr = FREELIST;
    Node_t *prev = NULL;
    while (curr != NULL)
    {
        // Ensure that chunk size is at least a page.
        if (curr->size < PAGESIZE)
        {
            // if not, move to next chunk
            prev = curr;
            curr = curr->next;
            continue;
        }

        // chop a page from end of chunk
        result = (uintptr_t)(curr->addr + curr->size - PAGESIZE);
        curr->size -= PAGESIZE;

        // If the chunk has no more free space
        if (curr->size == 0)
        {
            // ... and no previous node in the list
            if (prev == NULL)
            {
                // set the new head to be the next chunk
                FREELIST = curr->next;
            }
            else
            {
                // otherwise set the head's next to skip over the now empty chunk
                prev->next = curr->next;
            }
        }
        return result;
    }
    // return the phys address of the allocated memory.
    return result;
}

/**
 * Free a page of physical memory.
 * \param p is the physical address of the page to free, which must be page-aligned.
 */
void pmem_free(uintptr_t p)
{
    // add the page to the front of the freelist, as its own chunk
    Node_t *node = (Node_t *)(p + hhdm_addr);
    node->size = PAGESIZE;
    node->addr = p;
    node->next = FREELIST;
    FREELIST = node;
}

// Gets the level 4 page table
uintptr_t read_cr3()
{
    uintptr_t value;
    __asm__("mov %%cr3, %0"
            : "=r"(value));
    return value;
}

void write_cr3(uint64_t value)
{
    __asm__("mov %0, %%cr3"
            :
            : "r"(value));
}

/**
 * Translate a virtual address to its mapped physical address
 *
 * \param address     The virtual address to translate
 */
void translate(uintptr_t page_table, void *address)
{
    kprintf("Translating %p\n", address);
    
    uintptr_t cur_phys_table = page_table & ~(0xFFFLL);

    uintptr_t cur_virt_table = cur_phys_table + hhdm_addr;
    uint64_t cur_index;
    pt_entry_t cur_entry;

    // traverse through page tabel levels 4 through 1
    for (int i = 0; i < 4; i++)
    {
        // get page table level index from linear address bits
        cur_index = ((uint64_t)address >> (39 - (9 * i))) & 0x1FF;

        // entry at index
        cur_entry = *((pt_entry_t *)cur_virt_table + cur_index);

        kprintf("  Level %d (%d index of %p)\n", 4 - i, cur_index, cur_phys_table);

        // if present, print information stored in entry
        if (cur_entry.present == 1)
        {
            cur_phys_table = (cur_entry.address << 12);
            cur_virt_table = cur_phys_table + hhdm_addr;

            kprintf("    %s", cur_entry.user ? "user" : "kernel");
            kprintf("%s", cur_entry.writable ? " writable" : "");
            kprintf("%s", cur_entry.no_execute ? "" : " executable");
            kprintf(" -> %p\n", cur_phys_table);
        }
        else
        {
            kprintf("    not present\n");
            return;
        }
    }
    uint64_t offset = (uint64_t)address & 0xFFF;
    kprintf("%p maps to %p\n", address, cur_phys_table + offset);
}

/**
 * Notify the TLB that it should discard any cached translations for the mapping by reloading the cr3 register
 * Used to cause page faults when trying to access unmapped memory that was previously mapped
 * */
void invalidate_tlb(uintptr_t virtual_address)
{
    __asm__("invlpg (%0)" ::"r"(virtual_address)
            : "memory");
}

/**
 * Map a single page of memory into a virtual address space.
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to map into the address space, must be page-aligned
 * \param user Should the page be user-accessible?
 * \param writable Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if the mapping succeeded, or false if there was an error
 */
bool vm_map(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable)
{

    // Start at the top-level page table structure
    uintptr_t cur_phys_table = root;
    uintptr_t cur_virt_table = ptov(cur_phys_table);
    uint64_t cur_index;
    pt_entry_t *cur_entry;

    // traverse through page table levels 4 through 2.
    for (int i = 0; i < 3; i++)
    {
        // get page table level index from linear address bits
        cur_index = ((uint64_t)address >> (39 - (9 * i))) & 0x1FF;

        // entry at the index tells us physical address of next level page table
        cur_entry = &((pt_entry_t *)cur_virt_table)[cur_index];

        // If a page table doesn't exist
        if (cur_entry->present != 1)
        {
            // allocate space for a new table
            uintptr_t new_page = pmem_alloc();
            if (new_page == -1)
            {
                kprintf("Could not allocate new page");
                return false;
            }

            // initialize the table
            memset((void *)ptov(new_page), 0, PAGESIZE);

            // fill entry with permissions
            cur_entry->present = 1;
            cur_entry->user = 1;
            cur_entry->writable = 1;
            cur_entry->no_execute = 0;

            // point to the new table we just allocated
            cur_entry->address = new_page >> 12;
        }
        
        // move to next level page table for next iteration
        cur_phys_table = (cur_entry->address << 12);
        cur_virt_table = ptov(cur_phys_table);
    }

    // At the level one page table
    cur_index = ((uint64_t)address >> 12) & 0x1FF;
    cur_entry = &((pt_entry_t *)cur_virt_table)[cur_index];

    // If address already mapped, return
    if (cur_entry->present == 1)
    {
        kprintf("Address is already mapped\n");
        return false;
    }
    // fill entry with permissions
    cur_entry->present = 1;
    cur_entry->user = user;
    cur_entry->writable = writable;
    cur_entry->no_execute = !executable;

    invalidate_tlb(address);

    return true;
}

/**
 * Change the protections for a page in a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to update
 * \param user Should the page be user-accessible or kernel only?
 * \param writable Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if successful, or false if anything goes wrong (e.g. page is not mapped)
 */
bool vm_protect(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable)
{
    uintptr_t cur_phys_table = root;
    uintptr_t cur_virt_table = cur_phys_table + hhdm_addr;
    uint64_t cur_index;
    pt_entry_t cur_entry;

    for (int i = 0; i < 3; i++)
    {
        cur_index = ((uint64_t)address >> (39 - (9 * i))) & 0x1FF;
        cur_entry = *((pt_entry_t *)cur_virt_table + cur_index);
        if (cur_entry.present != 1)
        {
            return false;
        }
        cur_phys_table = (cur_entry.address << 12);
        cur_virt_table = cur_phys_table + hhdm_addr;
    }

    cur_index = ((uint64_t)address >> 12) & 0x1FF;
    cur_entry = ((pt_entry_t *)cur_virt_table)[cur_index];

    if (cur_entry.present != 1)
    {
        return false;
    }

    cur_entry.user = user;
    cur_entry.writable = writable;
    cur_entry.no_execute = !executable;

    invalidate_tlb(address);

    return true;
}

/**
 * Unmap a page from a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to unmap from the address space
 * \returns true if successful, or false if anything goes wrong
 */
bool vm_unmap(uintptr_t root, uintptr_t address)
{
    // TODO
    return true;
}

// Unmap everything in the lower half of an address space with level 4 page table at address root
void unmap_lower_half(uintptr_t root)
{
    // We can reclaim memory used to hold page tables, but NOT the mapped pages
    pt_entry_t *l4_table = (pt_entry_t *)ptov(root);
    for (size_t l4_index = 0; l4_index < 256; l4_index++)
    {

        // Does this entry point to a level 3 table?
        if (l4_table[l4_index].present)
        {

            // Yes. Mark the entry as not present in the level 4 table
            l4_table[l4_index].present = false;

            // Now loop over the level 3 table
            pt_entry_t *l3_table = (pt_entry_t *)ptov(l4_table[l4_index].address << 12);
            for (size_t l3_index = 0; l3_index < 512; l3_index++)
            {

                // Does this entry point to a level 2 table?
                if (l3_table[l3_index].present && !l3_table[l3_index].page_size)
                {

                    // Yes. Loop over the level 2 table
                    pt_entry_t *l2_table = (pt_entry_t *)ptov(l3_table[l3_index].address << 12);
                    for (size_t l2_index = 0; l2_index < 512; l2_index++)
                    {

                        // Does this entry point to a level 1 table?
                        if (l2_table[l2_index].present && !l2_table[l2_index].page_size)
                        {

                            // Yes. Free the physical page the holds the level 1 table
                            pmem_free(l2_table[l2_index].address << 12);
                        }
                    }

                    // Free the physical page that held the level 2 table
                    pmem_free(l3_table[l3_index].address << 12);
                }
            }

            // Free the physical page that held the level 3 table
            pmem_free(l4_table[l4_index].address << 12);
        }
    }

    // Reload CR3 to flush any cached address translations
    write_cr3(read_cr3());
}