#include <stdint.h>
#include <stddef.h>

#include "stivale2.h"
#include "util.h"
#include "kprint.h"
#include "memory.h"
#include "interrupts.h"
#include "pic.h"

void test_kgetc()
{
    kprintf("Testing kgetc\n");
    kprintf("Type 10 characters\n");
    for (int i = 0; i < 10; i++)
    {
        kprintf("%c", kgetc());
    }
}

void test_sys_calls()
{
    kprintf("Testing sys_read\n");
    kprintf("Type 5 characters\n");
    long rc;
    char buf[6];
    rc = syscall(SYS_READ, 0, buf, 5);
    if (rc <= 0)
    {
        kprintf("read failed\n");
    }
    else
    {
        buf[rc] = '\0';
        kprintf("read '%s'\n", buf);
    }

    kprintf("Testing sys_write\n");
    rc = syscall(SYS_WRITE, 1, "Hello world!\n", 13);
    if (rc <= 0)
    {
        kprintf("write failed\n");
    }
    else
    {
        kprintf("write succeeded\n");
    }
}

void test_translate()
{
    translate(read_cr3(), (void *)0x0);
}

void test_vm_map()
{
    uintptr_t root = read_cr3() & 0xFFFFFFFFFFFFF000;
    int *p = (int *)0x50004000;
    bool result = vm_map(root, (uintptr_t)p, false, true, false);
    if (result)
    {
        *p = 123;
        kprintf("Stored %d at %p\n", *p, p);
    }
    else
    {
        kprintf("vm_map failed with an error\n");
    }
}
