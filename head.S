/* head.S for bootstub to load protected mode kernel
 * Copyright (C) 2008 Alek Du <alek.du@intel.com>
 * 
 * Note. When FW hand-off control to bootstub, the CPU is already in protected
 * Mode with 1. GDT(8)=4G GDT(10)=4G
 *           2. CS=8, DS=ES=FS=GS=10
 *           3. Paging mode disabled
 *           4. Interrupt ENABLED
 */ 

/* When bootstub get control, the memory map in DRAM is like:
 *		~			~
 * 0x102000	|	initrd		| initrd will move to highest memory otherwise 
 *+bzImage size +-----------------------+ bzImage uncompressing will destory it
 *		|	bzImage		|
 * 0x102000	+-----------------------+
 *		|	boot stub	|
 * 0x101000	+-----------------------+
 *		|	free space	|
 *		|	used as stack	|
 * 0x100108	+-----------------------+
 *		|	initrd size	| MIC need to fill it according to bzImage size
 * 0x100104	+-----------------------+
 *		|	bzImage size	| MIC need to fill it according to initrd  size
 * 0x100100	+-----------------------+
 *		|	kernel cmdline	| MIC need to fill it
 * 0x100000	+-----------------------+
*/

#include "bootstub.h"

.text

.section ".text.head","ax",@progbits
	.globl _start

_start:
	cld
	cli
	/* setup stack, because we are heading off to "C" */
	movl $STACK_OFFSET, %esp
	/* after call bootstub, GDT was set (0x10 and 0x18) IDT was clear
	 * eax will store 32bit entry of bzImage
         */
	calll bootstub
	/* DS=ES=FS=GS=10 */
	movl $__BOOT_DS, %ebx	
	movl %ebx, %ds
	movl %ebx, %es
	movl %ebx, %fs
	movl %ebx, %gs
	movl %ebx, %ss
	ljmp $__BOOT_CS,$1f
1:	
	/* tell kernel where is boot_param */
	movl $(BOOT_PARAMS_OFFSET), %esi
	xor %ebp, %ebp
	xor %edi, %edi
	xor %ebx, %ebx
	
	jmpl *%eax    # Jump to the 32-bit entrypoint
	
