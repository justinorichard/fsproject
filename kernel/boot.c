#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "terminal.h"
#include "stivale2.h"
#include "util.h"
#include "kprint.h"
#include "memory.h"
#include "interrupts.h"
#include "pic.h"
#include "gdt.h"
#include "modules.h"
#include "testing.h"

#include "usermode_entry.h"

// headers for demo
#include "fs.h"

#define PAGESIZE 0x1000

// Reserve space for the stack
static uint8_t stack[8192];

// Added to cause page fault, not sure if needed.
static struct stivale2_tag unmap_null_hdr_tag = {
    .identifier = STIVALE2_HEADER_TAG_UNMAP_NULL_ID,
    .next = 0};

// Request a terminal from the bootloader
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        .next = (uintptr_t)&unmap_null_hdr_tag},
    .flags = 0};

// Declare the header for the bootloader
__attribute__((section(".stivale2hdr"), used)) static struct stivale2_header stivale_hdr = {
    // Use ELF file's default entry point
    .entry_point = 0,

    // Use stack (starting at the top)
    .stack = (uintptr_t)stack + sizeof(stack),

    // Bit 1: request pointers in the higher half
    // Bit 2: enable protected memory ranges (specified in PHDR)
    // Bit 3: virtual kernel mappings (no constraints on physical memory)
    // Bit 4: required
    .flags = 0x1E,

    // First tag struct
    .tags = (uintptr_t)&terminal_hdr_tag};

// Find a tag with a given ID
void *find_tag(struct stivale2_struct *hdr, uint64_t id)
{
  // Start at the first tag
  struct stivale2_tag *current = (struct stivale2_tag *)hdr->tags;

  // Loop as long as there are more tags to examine
  while (current != NULL)
  {
    // Does the current tag match?
    if (current->identifier == id)
    {
      return current;
    }

    // Move to the next tag
    current = (struct stivale2_tag *)current->next;
  }

  // No matching tag found
  return NULL;
}

typedef void *entry_fn_t();

void fs_demo() {
  kprintf("Open new file new_file.txt");
  int fd = fs_open("new_file.txt");
  if (!fs_write(fd, "nihao", 5)) {
    printf("write failed!\n");
  }
  fs_append(0, "zx\n", 3);

  fs_read(fd);
}

void _start(struct stivale2_struct *hdr)
{
  // Setup everything
  gdt_setup();
  idt_setup();
  pic_init();
  pic_unmask_irq(1);
  freelist_init(hdr);
  uintptr_t root = read_cr3() & ~0xFFFLL;
  unmap_lower_half(root);
  term_init();

  // Print a greeting
  usable_mem(hdr);
  kprintf("Hello Kernel!\n");
  fs_init();

  // demo fs
  fs_demo();

  // Pick an arbitrary location and size for the user-mode stack
  uintptr_t user_stack = 0x70000000000;
  // 70000008000
  size_t user_stack_size = 8 * PAGESIZE;

  // Map the user-mode-stack
  for (uintptr_t p = user_stack; p < user_stack + user_stack_size; p += 0x1000)
  {
    // Map a page that is user-accessible, writable, but not executable
    vm_map(root, p, true, true, false);
  }

  void *header_entry = load_module(hdr, "init");

  // And now jump to the entry point
  usermode_entry(USER_DATA_SELECTOR | 0x3,         // User data selector with priv=3
                 user_stack + user_stack_size - 8, // Stack starts at the high address minus 8 bytes
                 USER_CODE_SELECTOR | 0x3,         // User code selector with priv=3
                 (uintptr_t)header_entry);         // Jump to the entry point specified in the ELF file

    // We're done
  halt();
}