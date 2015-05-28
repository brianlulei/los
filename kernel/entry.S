
#include <include/mmu.h>
#include <include/memlayout.h>



#define RELOC(x) ((x) - KERNBASE)

.text

# '_start' specifies the ELF entry point. At this point, the virtual memory
# and paging haven't been enabled. The kernel binary is loaded at physical 
# address = 0x100000. We need to make the virtual address of '_start' symbol
# = it's *physical* address. But for other global symbols, we want to use the
# virtual addresses (> 0xF0000000). This is the trick to handle this.
.code32
.globl	_start
_start = RELOC(entry)

.globl entry
entry:
	movw	$0x1234, 0x472		# warm boot. why?

	# We haven't set up virtual memroy yet, so we're running from
	# the physical address the boot loader loaded the kernel at: 1MB
	# (plus a few bytes). However, the C code is linked to run at
	# KERNBASE + 1MB. Hence, we set up a trivial page directory that
	# tanslate virtual addresses [KERNBASE, KERNBASE+4MB) to physical
	# addresses[0, 4MB). This 4MB region is sufficient until we set up
	# our real page table later.

	# Load the *physical* address of entry_pgdir into cr3. entry_pgdir is
	# defined in entrypgdir.c.
	movl	$(RELOC(entry_pgdir)), %eax
	movl	%eax, %cr3						# load entrypgdir into control register cr3

	# Turn on paging
	movl	%cr0, %eax
	orl		$(CR0_PE|CR0_PG|CR0_WP), %eax
	movl	%eax, %cr0

	# Now paging is enabled, but we're still running at a low EIP
	# Jump up above KERNBASE before entering C code.
	movl	$relocated, %eax
	jmp		*%eax

relocated:

	# Clear the frame pointer register (EBP)
	# so that once we get into debugging C code,
	# stack backtraces will be terminated properly.
	movl	$0x0, %ebp		# nuke frame pointer

	# Set the stack pointer
	movl	$(bootstacktop), %esp

	# Now to C code
	call	i386_init

	# Should never get here, but in case we do, just spin
spin:
	jmp		spin

.data
############################################################
# boot stack
############################################################
	.p2align	PGSHIFT		# force page alignment
	.globl		bootstack
bootstack:
	.space		KSTKSIZE
	.globl		bootstacktop
bootstacktop:		
