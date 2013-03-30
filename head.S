/*
 * head.S for bootstub to load protected mode kernel
 *
 * Copyright (C) 2008-2010 Intel Corporation.
 * Author: Alek Du <alek.du@intel.com>
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

/*
 *
 * Note. When FW hand-off control to bootstub, the CPU is already in protected
 * Mode with 1. GDT(8)=4G GDT(10)=4G
 *           2. CS=8, DS=ES=FS=GS=10
 *           3. Paging mode disabled
 *           4. Interrupt ENABLED
 *
 * When bootstub get control, the memory map in DRAM is like:
 *		~			~
 * 0x1102000	|	initrd		| initrd will move to highest memory otherwise 
 *+bzImage size +-----------------------+ bzImage uncompressing will destory it
 *		|	bzImage		|
 * 0x1102000	+-----------------------+
 *		|	boot stub	|
 * 0x1101000	+-----------------------+
 *		|	free space	|
 *		|	used as stack	|
 * 0x1100110	+-----------------------+
 *		|	SPI0 or SPI1	| MIC need to fill it:
 *		|			| 0x0: SPI0, 0x1: SPI1
 * 0x110010c	+-----------------------+
 *		|   boot stub spi uart	| MIC need to fill it:
 *		|   suppression flag	| 0x1 suppression, 0x0 default
 * 0x1100108	+-----------------------+
 *		|	initrd size	| MIC need to fill it: initrd file size
 * 0x1100104	+-----------------------+
 *		|	bzImage size	| MIC need to fill it: bzImage file size
 * 0x1100100	+-----------------------+
 *		|	kernel cmdline	| MIC need to fill it
 * 0x1100000	+-----------------------+
*/

#include "bootstub.h"

.text

.section ".text.head","ax",@progbits
	.globl _start

_start:
	cld
	cli

	/* Set our own GDT and IDT, don't derive from IAFW */
	lgdtl	%cs:gdtr
	lidtl	%cs:idtr

	/* Load segment registers per protected mode kernel entry requirement:
	 * CS=0x10,
	 * DS=ES=SS=FS=GS=0x18
	 */
	ljmp	$__BOOT_CS, $1f
1:
	movl $__BOOT_DS, %ebx	
	movl %ebx, %ds
	movl %ebx, %es
	movl %ebx, %fs
	movl %ebx, %gs
	movl %ebx, %ss

	/* setup stack, because we are heading off to "C" */
	movl $STACK_OFFSET, %esp

	/* bootstub() returns 32bit entry address of bzImage, stored in eax */
	calll bootstub

	/* tell kernel where is boot_param */
	movl $(BOOT_PARAMS_OFFSET), %esi
	xor %ebp, %ebp
	xor %edi, %edi
	xor %ebx, %ebx

	/* Jump to the 32-bit entrypoint */
	jmpl *%eax

	.balign 8
gdt:
        .quad   0
        .quad   0
        .quad   GDT_ENTRY(0xc09b, 0, 0xfffff)
        .quad   GDT_ENTRY(0xc093, 0, 0xfffff)
gdtr:
        .word   4*8-1
        .long   gdt

	.balign 8
idt:
	.quad	0
	.quad	0
idtr:
	.word	2*8-1
	.long	idt
