/* define bootstub constrains here, like memory map etc. 
 */

#ifndef _BOOT_STUB_HEAD
#define _BOOT_STUB_HEAD

#define CMDLINE_OFFSET		0x1100000
#define BZIMAGE_SIZE_OFFSET	0x1100100
#define INITRD_SIZE_OFFSET	0x1100104
#define SPI_UART_SUPPRESSION	0x1100108
#define SPI_TYPE		0x110010c /*0:SPI0  1:SPI1*/
#define STACK_OFFSET		0x1101000
#define BZIMAGE_OFFSET		0x1102000

#define SETUP_HEADER_OFFSET (BZIMAGE_OFFSET + 0x1F1)
#define SETUP_HEADER_SIZE (0x0202 + *(unsigned char*)(0x0201+BZIMAGE_OFFSET))
#define BOOT_PARAMS_OFFSET 0x8000
#define SETUP_SIGNATURE 0x5a5aaa55

#define GDT_ENTRY_BOOT_CS       2
#define __BOOT_CS               (GDT_ENTRY_BOOT_CS * 8)

#define GDT_ENTRY_BOOT_DS       (GDT_ENTRY_BOOT_CS + 1)
#define __BOOT_DS               (GDT_ENTRY_BOOT_DS * 8)

#define GDT_ENTRY(flags, base, limit)           \
        (((u64)(base & 0xff000000) << 32) |     \
         ((u64)flags << 40) |                   \
         ((u64)(limit & 0x00ff0000) << 32) |    \
         ((u64)(base & 0x00ffffff) << 16) |     \
         ((u64)(limit & 0x0000ffff)))

#endif
