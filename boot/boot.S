/*
 *  boot.S
 *  Copyright (C) 2015  Lei Lu (lulei.wm@gmail.com)
 */

#include <include/mmu.h>

.set CR0_PE_ON,			0x01
.set PROT_MODE_CSEG,	0x08 
.set PROT_MODE_DSEG,	0x10


.text
.globl start
.code16			# 16-bit mode
start:
	cli			# Disable maskable hardware interrupts, entering protected mode
				# all interrupts work in real mode will not be available in protected mode
	cld			# String operations increment

	# BIOS does not guarantee anything about the contents of %ds, %es and %ss
	# Set up the important data segment registers (DS, ES, SS)
	xorw	%ax, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss

	# Enable A20:
	# A20 is a bus gate, after power-up it is closed, enable it gives the ability to access
	# the memory beyond 1MB
	# For backwards compatibility with the earlies PCs, physical address line 20 is closed,
	# so that addresses higher than 1MB wrap around to zero by default. This code undoes this.
enable_a20.1:
	inb		$0x64, %al		# Read status register
	testb	$0x2, %al		# 0: Input buffer empty, can be written
							# 1: Input buffer full, don't write yet
	jnz		enable_a20.1
	movb	$0xdf, %al		# command 0xdf: enable a20
	outb	%al, $0x64		# send command to controller

	# load GDT descriptor into register GTDR
	lgdt	gdtdesc

	# Switch from real mode to protected mode
	movl	%cr0, %eax
	orl		$CR0_PE_ON, %eax
	movl	%eax, %cr0

	# Jump to next instruction, but in 32-bit code segment
	# Switches processor into 32-bit mode.
	ljmp	$PROT_MODE_CSEG, $protcseg

.code32
protcseg:
	# Set up the protectd-mode data segment registers
	movw	$PROT_MODE_DSEG, %ax	# Our data segment selector
	movw	%ax, %ds				# -> DS: Data Segment
	movw	%ax, %es				# -> ES: Extra Segment
	movw	%ax, %fs				# -> FS
	movw	%ax, %gs				# -> GS
	movw	%ax, %ss				# -> SS: Stack Segment

	# Set up the stack pointer and call into C.
	movl	$start, %esp
	call bootmain

	#If bootmain returns (it sholdn't), loop.
spin:
	jmp		spin

.p2align 2								# align on a 4-byte boundary
gdt:
	SEG_NULL							# null descriptor
	SEG(STA_X|STA_R, 0x0, 0xffffffff)	# code segment
	SEG(STA_W, 0x0, 0xffffffff)			# data segment

gdtdesc:
	.word .-gdt-1			# sizeof(gdt) -1
	.long gdt				# address of gdt

.org    0x1fe,  0x90
.word   0xaa55
