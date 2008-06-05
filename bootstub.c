/*
 * bootstub 32 bit entry setting routings 
 * Copyright (C) 2008, Alek Du <alek.du@intel.com> Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "types.h"
#include "bootstub.h"
#include "bootparam.h"

struct gdt_ptr {
        u16 len;
        u32 ptr;
} __attribute__((packed));

static void setup_gdt(void)
{
        static const u64 boot_gdt[] __attribute__((aligned(16))) = {
                /* CS: code, read/execute, 4 GB, base 0 */
                [GDT_ENTRY_BOOT_CS] = GDT_ENTRY(0xc09b, 0, 0xfffff),
                /* DS: data, read/write, 4 GB, base 0 */
                [GDT_ENTRY_BOOT_DS] = GDT_ENTRY(0xc093, 0, 0xfffff),
        };
        static struct gdt_ptr gdt;

        gdt.len = sizeof(boot_gdt)-1;
        gdt.ptr = (u32)&boot_gdt;

        asm volatile("lgdtl %0" : : "m" (gdt));
}

static void setup_idt(void)
{
        static const struct gdt_ptr null_idt = {0, 0};
        asm volatile("lidtl %0" : : "m" (null_idt));
}

static void *memcpy(void *dest, const void *src, size_t count)
{
        char *tmp = dest;
        const char *s = src;

        while (count--)
                *tmp++ = *s++;
        return dest;
}

static void *memset(void *s, int c, size_t count)
{
        char *xs = s;
 
        while (count--)
                *xs++ = c; 
        return s;
}

static size_t strnlen(const char *s, size_t maxlen)
{
        const char *es = s;
        while (*es && maxlen) {
                es++;
                maxlen--;
        }

        return (es - s);
}


static void setup_boot_params(struct boot_params *bp, struct setup_header *sh)
{
	memset(bp, 0, sizeof (struct boot_params));
	bp->screen_info.orig_video_mode = 0;
	bp->screen_info.orig_video_lines = 0;
	bp->screen_info.orig_video_cols = 0;
	bp->alt_mem_k = 128*1024; // hard coded 128M mem here, since SFI will override it
	memcpy(&bp->hdr, sh, sizeof (struct setup_header));
	bp->hdr.cmd_line_ptr = CMDLINE_OFFSET;
	bp->hdr.cmdline_size = strnlen((const char *)CMDLINE_OFFSET,256);
	bp->hdr.type_of_loader = 0xff; //bootstub is unknown bootloader for kernel :)
	bp->hdr.ramdisk_size = *(u32 *)INITRD_SIZE_OFFSET;
	bp->hdr.ramdisk_image = (bp->alt_mem_k*1024 - bp->hdr.ramdisk_size) & 0xFFFFF000;
	memcpy((u8*)bp->hdr.ramdisk_image, (u8 *)BZIMAGE_OFFSET + *(u32 *)BZIMAGE_SIZE_OFFSET, bp->hdr.ramdisk_size);
}

static int get_32bit_entry(unsigned char *ptr)
{
	while (1){
		if (*(u32 *)ptr == SETUP_SIGNATURE && *(u32 *)(ptr+4) == 0)
			break;
		ptr++;
	}
	ptr+=4;
	return (((unsigned int)ptr+511)/512)*512;
}

/* 
 *	SPI0 debug printk support
 * LNC can only use SPI0(0xff128000) for debug output, and it can only
 * print out single string, not supporting variable parameter
 */

static spi_inited = 0;
static struct mrst_spi_reg *pspi = 0;

static void spi_init()
{
	u32 ctrlr0;

	pspi = (struct mrst_spi_reg *)MRST_REGBASE_SPI0;

	/* disable SPI controller first */
	pspi->ssienr = 0x0;

	/* set control param, 16 bits, transmit only mode */
	ctrlr0 = pspi->ctrlr0;

	ctrlr0 &= 0xfcc0;
	ctrlr0 |= (0xf | (FRF_SPI << SPI_FRF_OFFSET)
		       | (TMOD_TO << SPI_TMOD_OFFSET));
	pspi->ctrlr0 = ctrlr0;
	
	/* set a default baud rate, 115200 */
	/* feng, need make sure SPIC and MAXIM3110 match */
	//spi_enable_clk(32);
	pspi->baudr = 0x2;

	/* need set the transmit threshhol? */
	pspi->txftlr = 0x3;

	/* disable all INT for early phase */
	pspi->imr &= 0xffffff00;
	
	/* select one slave SPI device */
	pspi->ser = 0x1;

	/* enable the HW, this should be the last step for HW init */
	pspi->ssienr |= 0x1;

	spi_inited = 1;
}

/* set the ratio rate, INT */
static void max3110_write_config(void)
{
	u16 config;

	/* 115200, TM not set, no parity, 8bit word */
	config = 0xc000; 	
	pspi->dr[0] = config;
}

/* transfer char to a eligibal word and send to max3110 */
static int max3110_write_data(char c)
{
	u16 data;
	u8  config;

	data = 0x8000 | c;
	pspi->dr[0] = data;
}

/* slave select should be called in the read/write function */
static int spi_max3110_putc(char c)
{
	unsigned int timeout;
	u32 sr;
	u32 test;

#define MRST_SPI_TIMEOUT	0x200000

	/* read RX FIFO out if there is any */
	while ((pspi->sr & SR_RF_NOT_EMPT) && pspi->rxflr ) {
		timeout = MRST_SPI_TIMEOUT;
		while (timeout--) {
			 if (!(pspi->sr & SR_BUSY))
				break;
		}

		if (timeout == 0xffffffff)
			return -1;
		test = pspi->dr[0];
	}

	timeout = MRST_SPI_TIMEOUT;
	/* early putc need make sure the TX FIFO is empty */
	while (timeout--) {
		sr = pspi->sr;
		if ( (sr & SR_BUSY) || !(sr & SR_TF_EMPT))
			continue;
		else
			break;
	}

	if (timeout == 0xffffffff) {
		return -1;
	}

	max3110_write_data(c);

	return 0;
}


void bs_spi_printk(const char *str)
{
	u32 ctrlr0;

	if (!spi_inited)
		spi_init();

	if (!str)
		return;

	/* 
	 * here we assume only 1 write_config is enough,
	 * if not will call it for each putc 
	 */
	max3110_write_config();

	while (*str) {
		if (*str == '\n')
			spi_max3110_putc('\r');
		spi_max3110_putc(*str++);
	}
}

int bootstub(void)
{
	bs_spi_printk("Bootstub Version: \n");
	setup_idt();
	setup_gdt();
	setup_boot_params((struct boot_params *)BOOT_PARAMS_OFFSET, 
		(struct setup_header *)SETUP_HEADER_OFFSET);
	return get_32bit_entry((unsigned char *)BZIMAGE_OFFSET);
}
