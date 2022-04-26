#include <stdint.h>
#include <stddef.h>
#include <mem.h>
#include <stdbool.h>
#include <string.h>

#include "kprint.h"
#include "util.h"
#include "pic.h"
#include "port.h"
#include "interrupts.h"
#include "io.h"
#include "gdt.h"
#include "memory.h"
#include "modules.h"

#define PAGESIZE 0x1000

#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_NONE 8
#define PROT_EXEC 4

#define MAP_PRIVATE 2

#define MMAP_MEM_START 0x80000000000
#define MOD_BUF_LEN 100


// Struct tells the interrupt handler about what was happening when the interrupt occured.
typedef struct interrupt_context
{
    uintptr_t ip;
    uint64_t cs;
    uint64_t flags;
    uintptr_t sp;
    uint64_t ss;
} __attribute__((packed)) interrupt_context_t;


// Handlers for our different interrupts


__attribute__((interrupt)) void default_handler(interrupt_context_t *ctx)
{
    kprintf("Default interrupt handler\n");
    halt();
}

__attribute__((interrupt)) void default_handler_ec(interrupt_context_t *ctx, uint64_t ec)
{
    kprintf("Default interrupt handler (ec=%d)\n");
    halt();
}

// Handler index 0
__attribute__((interrupt)) void divide_handler(interrupt_context_t *ctx)
{
    kprintf("Divide interrupt handler\n");
    halt();
}

// Handler index 13
__attribute__((interrupt)) void general_protection_handler(interrupt_context_t *ctx, uint64_t ec)
{
    kprintf("General Protection interrupt handler (ec=%d)\n", ec);
    halt();
}

// Handler index 14
__attribute__((interrupt)) void page_fault_handler(interrupt_context_t *ctx, uint64_t ec)
{
    kprintf("Page Fault interrupt handler (ec=%d)\n", ec);
    halt();
}

// Keyboard Interupts #######################

// Keys
char *table = "**1234567890-="
              "\b\tqwertyuiop[]"
              "\n*asdfghjkl;'`*\\"
              "zxcvbnm,./*"
              "** *";

char *table_caps = "..!@#$%^&*()_+"
                   "\b\tQWERTYUIOP{}"
                   "\n.ASDFGHJKL:\"~.|"
                   "ZXCVBNM<>?."
                   ".. .";

// Circular buffer
char iobuffer[IO_BUFFER_SIZE];
volatile int read_val = 0, write_val = 0;
volatile int iobuffer_count = 0;
int left_shift_flag, right_shift_flag = 0;
// Handler index 33
__attribute__((interrupt)) void keyboard_handler(interrupt_context_t *ctx)
{
    uint64_t code = (uint64_t)inb(0x60);
    // Prints out the typed keycode
    if (code == L_SHIFT_PRESSED)
    {
        left_shift_flag = 1;
    }
    else if (code == R_SHIFT_PRESSED)
    {
        right_shift_flag = 1;
    }
    else if (code == L_SHIFT_RELEASED)
    {
        left_shift_flag = 0;
    }
    else if (code == R_SHIFT_RELEASED)
    {
        right_shift_flag = 0;
    }
    else if (code > 0x3A)
    {
        // If code represents a release, don't process it.
    }
    else if (iobuffer_count == IO_BUFFER_SIZE)
    {
        kprintf("Buffer is full");
    }
    else if (left_shift_flag == 1 || right_shift_flag == 1)
    {
        iobuffer[write_val++] = table_caps[code];
        write_val %= IO_BUFFER_SIZE;
        iobuffer_count++;
    }
    else
    {
        iobuffer[write_val++] = table[code];
        write_val %= IO_BUFFER_SIZE;
        iobuffer_count++;
    }

    outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * Read one character from the keyboard buffer. If the keyboard buffer is empty this function will
 * block until a key is pressed.
 *
 * \returns the next character input from the keyboard
 */
char kgetc()
{
    while (iobuffer_count == 0)
    {
    }
    char res = iobuffer[read_val++];
    read_val %= IO_BUFFER_SIZE;
    iobuffer_count--;
    return res;
}

// Called when a system call is issued.
int64_t syscall_handler(uint64_t nr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5)
{
    // Prints system call and parameters
    // kprintf("syscall %d: %d, %d, %d, %d, %d, %d\n", nr, arg0, arg1, arg2, arg3, arg4, arg5);

    switch (nr)
    {
    case SYS_READ:
        return SYS_read((int)arg0, (char *)arg1, (int)arg2);
    case SYS_WRITE:
        return SYS_write((int)arg0, (char *)arg1, (int)arg2);
    case SYS_MMAP:
        return SYS_mmap((void *)arg0, (size_t)arg1, (int)arg2, (int)arg3,
                        (int)arg4, (uint64_t)arg5);
    case SYS_EXEC:
        return SYS_exec((char *)arg0);
    case SYS_EXIT:
        return SYS_exit();
    }
    return 123;
}

extern int64_t syscall(uint64_t nr, ...);
extern void syscall_entry();

/**
 * Read system call, read from keyboard
 * \param file_descriptor
 * \param buffer to read into
 * \param length number of bytes to read
 */
long SYS_read(int file_descriptor, char *buffer, int length)
{
    // read keyboard input using standard input
    if (file_descriptor != 0)
    {
        return -1;
    }


    int i = 0;
    // read length number of characters from the keyboard buffer,
    //      and write them to the given buffer
    while (i < length)
    {
        char cur_char = kgetc();
        if (cur_char == '\b' && i > 0)
        {
            i--;
        }
        else if (cur_char != '\b')
        {
            buffer[i++] = cur_char;
        }
    }
    return i;
}


/**
 * Write system call, write to terminal
 * \param file_descriptor
 * \param buffer to write from
 * \param length number of bytes to write
 */
long SYS_write(int file_descriptor, char *buffer, int length)
{
    // write to terminal using standard output (1) or standard error (2)
    if ((file_descriptor != 1) && (file_descriptor != 2))
    {
        return -1;
    }

    // print each character from the given buffer
    for (int i = 0; i < length; i++)
    {
        kprintf("%c", buffer[i]);
    }
    return length;
}

// Change the null stuff to where mmap is
uintptr_t mem_start = MMAP_MEM_START;
long SYS_mmap(void *addr, size_t length, int prot, int flags,
              int fd, uint64_t offset)
{

    // Get top level page table
    uintptr_t root = read_cr3() & ~0xFFFLL;

    uintptr_t page_align_addr;
    bool rc;
    // If addr is specified
    if (addr != NULL)
    {

        page_align_addr = (uintptr_t)addr & ~0xFFFLL;

        size_t first_len;
        if (page_align_addr == (uintptr_t)addr)
        {
            first_len = PAGESIZE;
        }
        else
        {
            first_len = PAGESIZE - ((uintptr_t)addr - page_align_addr);
        }

        // rc = vm_map(root, page_align_addr, prot & PROT_READ, prot & PROT_WRITE, prot & PROT_EXEC);
        rc = vm_map(root, page_align_addr, true, true, true);
        if (!rc)
        {
            kprintf("Panic, vm_map didn't map the modules\n");
        }

        length -= first_len;
        page_align_addr += PAGESIZE;
    }
    // If address is not specified choose mem address
    else
    {
        addr = (void *)mem_start;
        page_align_addr = mem_start;
        int pages = length / PAGESIZE;
        if (length % PAGESIZE != 0)
        {
            pages++;
        }
        mem_start += pages * PAGESIZE;
    }

    while (length > 0)
    {
        rc = vm_map(root, page_align_addr, prot & PROT_READ, prot & PROT_WRITE, prot & PROT_EXEC);
        if (!rc)
        {
            kprintf("Panic, vm_map didn't map the modules\n");
        }
        length -= PAGESIZE;
        page_align_addr += PAGESIZE;
    }

    return (long)addr;
}

long SYS_exec(char *module)
{

    char kernel_mod[MOD_BUF_LEN];
    strcpy(kernel_mod, module);

    // Get top level page table
    uintptr_t root = read_cr3() & ~0xFFFLL;
    unmap_lower_half(root);
    mem_start = MMAP_MEM_START;

    entry_fn_t *func = (entry_fn_t *)load_module(hdr_global, kernel_mod);

    func();
    return 1;
}

long SYS_exit()
{
    // Get top level page table
    uintptr_t root = read_cr3() & ~0xFFFLL;
    unmap_lower_half(root);
    mem_start = MMAP_MEM_START;

    entry_fn_t *func = (entry_fn_t *)load_module(hdr_global, "init");
    func();
    return 1;
}

// system call stuff ^^^^^^^^^^^^^^^^^^^^^^^^^^

// Every interrupt handler must specify a code selector. We'll use entry 5 (5*8=0x28), which
// is where our bootloader set up a usable code selector for 64-bit mode.
#define IDT_CODE_SELECTOR 0x28

// IDT entry types
#define IDT_TYPE_INTERRUPT 0xE
#define IDT_TYPE_TRAP 0xF

// A struct the matches the layout of an IDT entry
typedef struct idt_entry
{
    uint16_t offset_0;
    uint16_t selector;
    uint8_t ist : 3;
    uint8_t _unused_0 : 5;
    uint8_t type : 4;
    uint8_t _unused_1 : 1;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t offset_1;
    uint32_t offset_2;
    uint32_t _unused_2;
}
__attribute__((packed)) idt_entry_t;

// Make an IDT
idt_entry_t idt[256];

/**
 * Set an interrupt handler for the given interrupt number.
 *
 * \param index The interrupt number to handle
 * \param fn    A pointer to the interrupt handler function
 * \param type  The type of interrupt handler being installed.
 *              Pass IDT_TYPE_INTERRUPT or IDT_TYPE_TRAP from above.
 */
void idt_set_handler(uint8_t index, void *fn, uint8_t type)
{
    idt[index].type = type;
    idt[index].present = 1;
    // Run the handler in given mode
    idt[index].dpl = 3;
    idt[index].ist = 0;
    idt[index].selector = KERNEL_CODE_SELECTOR;
    //   handler
    idt[index].offset_0 = (uint16_t)((uint64_t)fn & 0xffff);
    idt[index].offset_1 = (uint16_t)(((uint64_t)fn & 0xffff0000) >> 16);
    idt[index].offset_2 = (uint32_t)(((uint64_t)fn & 0xffffffff00000000) >> 32);
}

// This struct is used to load an IDT once we've set it up
typedef struct idt_record
{
    uint16_t size;
    void *base;
} __attribute__((packed)) idt_record_t;

/**
 * Initialize an interrupt descriptor table, set handlers for standard exceptions, and install
 * the IDT.
 */
void idt_setup()
{
    // Zero out the IDT using memset
    memset(idt, 0, 256 * sizeof(idt_entry_t));

    // Set handlers for the standard exceptions
    idt_set_handler(0, divide_handler, IDT_TYPE_TRAP);
    idt_set_handler(1, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(2, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(3, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(4, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(5, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(6, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(7, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(8, default_handler_ec, IDT_TYPE_TRAP);
    idt_set_handler(9, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(10, default_handler_ec, IDT_TYPE_TRAP);
    idt_set_handler(11, default_handler_ec, IDT_TYPE_TRAP);
    idt_set_handler(12, default_handler_ec, IDT_TYPE_TRAP);
    idt_set_handler(13, general_protection_handler, IDT_TYPE_TRAP);
    idt_set_handler(14, page_fault_handler, IDT_TYPE_TRAP);
    idt_set_handler(15, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(16, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(17, default_handler_ec, IDT_TYPE_TRAP);
    idt_set_handler(18, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(19, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(20, default_handler, IDT_TYPE_TRAP);
    idt_set_handler(21, default_handler_ec, IDT_TYPE_TRAP);

    idt_set_handler(IRQ1_INTERRUPT, keyboard_handler, IDT_TYPE_TRAP);

    idt_set_handler(0x80, syscall_entry, IDT_TYPE_TRAP);

    // Install the IDT
    idt_record_t record = {
        .size = sizeof(idt),
        .base = idt};
    __asm__("lidt %0" ::"m"(record));
}