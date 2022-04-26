#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <mem.h>

#include "stivale2.h"
#include "memory.h"
#include "util.h"
#include "kprint.h"
#include "modules.h"

void *find_tag(struct stivale2_struct *hdr, uint64_t id);

// Modified typedefs pulled from here
// https://opensource.apple.com/source/lldb/lldb-76/source/Plugins/ObjectFile/ELF/ELFHeader.h
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;
typedef uint64_t Elf64_Size;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

// Elf header and program header struct pulled from here
// https://uclibc.org/docs/elf-64-gen.pdf
#define EI_NIDENT 16
#define PT_LOAD 1
#define EXEC_FLAG 0x1
#define WRITE_FLAG 0x2
#define READ_FLAG 0x4

typedef struct
{
    unsigned char e_ident[EI_NIDENT]; /* ELF identification */
    Elf64_Half e_type;                /* Object file type */
    Elf64_Half e_machine;             /* Machine type */
    Elf64_Word e_version;             /* Object file version */
    Elf64_Addr e_entry;               /* Entry point address */
    Elf64_Off e_phoff;                /* Program header offset */
    Elf64_Off e_shoff;                /* Section header offset */
    Elf64_Word e_flags;               /* Processor-specific flags */
    Elf64_Half e_ehsize;              /* ELF header size */
    Elf64_Half e_phentsize;           /* Size of program header entry */
    Elf64_Half e_phnum;               /* Number of program header entries */
    Elf64_Half e_shentsize;           /* Size of section header entry */
    Elf64_Half e_shnum;               /* Number of section header entries */
    Elf64_Half e_shstrndx;            /* Section name string table index */
} Elf64_Ehdr;

typedef struct
{
    Elf64_Word p_type;
    Elf64_Word p_flags;
    Elf64_Off p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

void print_modules(struct stivale2_struct *hdr)
{
    struct stivale2_struct_tag_modules *modules_tag = find_tag(hdr, STIVALE2_STRUCT_TAG_MODULES_ID);
    uint64_t module_count = modules_tag->module_count; // Count of loaded modules

    for (int i = 0; i < module_count; i++)
    {
        struct stivale2_module module = modules_tag->modules[i];
        kprintf("%s %p - %p\n", module.string, module.begin, module.end);
    }
}

void *load_module(struct stivale2_struct *hdr, char *str)
{
    uintptr_t root = read_cr3() & ~0xFFFLL;
    struct stivale2_struct_tag_modules *modules_tag = find_tag(hdr, STIVALE2_STRUCT_TAG_MODULES_ID);
    uint64_t module_count = modules_tag->module_count; // Count of loaded modules

    // For each module
    for (int i = 0; i < module_count; i++)
    {
        // Get the header and program header offset
        struct stivale2_module module = modules_tag->modules[i];

        if (strcmp(module.string, str))
        {
            continue;
        }

        Elf64_Ehdr elf_header = *(Elf64_Ehdr *)module.begin;
        Elf64_Phdr *program_header_start = (Elf64_Phdr *)(module.begin + elf_header.e_phoff);
        for (int i = 0; i < elf_header.e_phnum; i++)
        {
            Elf64_Phdr program_header = program_header_start[i];

            if ((program_header.p_type != PT_LOAD) || (program_header.p_memsz == 0))
            {
                continue;
            }

            // The location and size of the bytes to be copied
            Elf64_Off file_offset = program_header.p_offset;
            Elf64_Xword byte_size = program_header.p_memsz;

            // Virtual address to map memory to
            Elf64_Addr virtual_addr_start = program_header.p_vaddr & ~0xFFF;

            // Flags
            Elf64_Word flags = program_header.p_flags;

            bool rc = vm_map(root, virtual_addr_start, true, true, true);
            if (!rc)
            {
                kprintf("Panic, vm_map didn't map the modules\n");
                halt();
            }

            memcpy((void *)program_header.p_vaddr, (void *)(module.begin + file_offset), byte_size);
            vm_protect(root, virtual_addr_start, true, (flags & PF_W), (flags & PF_X));
        }

        return (void *)elf_header.e_entry;
    }
    return (void *)-1;
}

void load_init(struct stivale2_struct *hdr)
{
    entry_fn_t *func = (entry_fn_t *)load_module(hdr, "init");
    func();
    return;
}
