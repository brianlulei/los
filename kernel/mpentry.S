#include <include/mmu.h>
#include <include/memlayout.h>

/*****************************************************************
 * Entry point for APs
 *****************************************************************/

/*****************************************************************
 * Each non-boot CPU ("AP") is started up in response to a STARTUP
 * IPI from the boot CPU. Section B.4.2 of the Multi-Processor
 * Specification says that the AP will start in real mode with CS:IP
 * set to XY00:0000, where XY is an 8-bit valuesent with the STARTUP.
 * Thus this code must start at a 40960-byte boundary.
 *
 * Because this code sets DS to zero, it must run from an address in
 * the low 2^16 bytes of physical memory.
 *
 * boot_aps() (in kernel/init.c) copies this code to MPENTRY_PADDR
 * (which satisfies the above restrictions). Then, for each AP, it
 * stores the address of the pre-allocated per-store stack in 
 * mpentry_kstack, sends the STARTUP IPI, and waits for this code
 * to acknowledge that it has started (which  happens in mp_main
 * in kernel/init.c).
 *
 * This code is similar to boot/boot.S except that
 *	- it does not need to enable A20
 *	- it uses MPBOOTPHYS to calculate absolute addresses of its
 *	  symbols, rater than relying on the linker to fill them
 *****************************************************************/

#define RELOC(x)		((x) - KERNBASE)
#define MPBOOTPHYS(s)	((s) - mpentry_start + MPENTRY_PADDR)

.set PROT_MODE_CSEG, 0x08	# kernel code segment selector
.set PROT_MODE_DSEG, 0x10	# kernel data segment selector

.code16
.globl	mpentry_start
mpentry_start:
	cli		# disable maskable hardware interrrupts, entering protected mode

	# Set up important data segment registers (DS, ES, SS)
	xorw	%ax,	%ax
	movw	%ax,	%ds
	movw	%ax,	%es
	movw	%ax,	%ss

	# load GDT descriptor into register GTDR
	lgdt	MPBOOTPHYS(gdtdesc)
	
	# Switch from real mode to protected mode
	movl	%cr0,	%eax
	orl		$CR0_PE,%eax
	movl	%eax,	%cr0

	# Jump to next instruction, but in 32-bit code segment
	# Switches processor into 32-bit mode.
	ljmp	$PROT_MODE_CSEG, $(MPBOOTPHYS(start32))

.code32
start32:
	movw	$PROT_MODE_DSEG,	%ax
	movw	%ax,	%ds
	movw	%ax,	%es
	movw	%ax,	%ss
	movw	$0,		%ax
	movw	%ax,	%fs
	movw	%ax,	%gs

	# Set up initial page table. We cannot use kern_pgdir yet because
	# we are still running at low EIP (don't quite understand, why?)
	movl	$(RELOC(entry_pgdir)),	%eax
	movl	%eax,	%cr3

	# Turn on paging
	movl	%cr0,	%eax
	orl		$(CR0_PE|CR0_PG|CR0_WP), %eax
	movl	%eax,	%cr0	
	
	# Switch to the per-cpu stack allocated in boot_aps()
	movl	mpentry_kstack, %esp
	movl	$0x0,	%ebp	# Nuke frame pointer

	# Call mp_main(). (Why indirect call?)
	movl	$mp_main, %eax
	call	*%eax

	# #If mp_main returns (it sholdn't), loop.
spin:
	jmp		spin

# Bootstrap GDT
.p2align 2								# Force 4 byte alignment
gdt:
	SEG_NULL							# Null descriptor
	SEG(STA_X|STA_R, 0x0, 0xffffffff)	# Code segment
	SEG(STA_W, 0x0, 0xffffffff)			# Data segment

gdtdesc:
	.word .-gdt-1						# Sizeof(gdt) - 1
	.long MPBOOTPHYS(gdt)				# address of gdt

.globl	mpentry_end
mpentry_end:
	nop
