#include <include/mmu.h>
#include <include/memlayout.h>

/* Page fault upcall entry point. */

/* This is where we ask the kernel to redirect us to whenever we cause
 * a page fault in user space (see the call to sys_set_pgfault_handler
 * in pgfault.c).
 *
 * When a page fault actually occurs, the kernel switches our ESP to 
 * point to the user excetion stack if we're not already on the user
 * exception stack, and then it pushes a UTrapframe onto our user
 * exception stack:
 *
 *	trap-time esp
 *	trap-time eflags
 *	trap-time eip
 *	utf_regs.reg_eax
 *	...
 *	utf_regs.reg_esi
 *	utf_regs.reg_edi
 *	utf_err (error code)
 *	utf_fault_va		<-- %esp
 *
 * If this is a recursive fault, the kernel will reserve for us a
 * blank word above the trap-time esp for scratch work when we unwind
 * the recursive call.
 *
 * We then have to call up to the appropriate page fault handler in C
 * code, pointed to by the global variable '_pgfault_handler'.
 */

.text
.global _pgfault_upcall
_pgfault_upcall:
	// Call the C page fault handler
	pushl	%esp					// function argument: pointer to UTF
	movl	_pgfault_handler, %eax
	call	*%eax					// page fault is handled by calling handler
	addl	$4, %esp				// pop function argument

	// Now the C page fault handler has returned and you must return
	// to the trap time state.
	// Push trap-time %eip onto the trap-time stack.
	//
	// Explanation:
	//	We must prepare the trap-time stack for our eventual return to
	//	re-execute the instruction that faulted.
	//	Unfortunately, we can't return directly from the exception stack:
	//	We can't call 'jmp', since that requires that we load the address
	//	into a register, and all registers must have their trap-time
	//	values after the return.
	//	We can't call 'ret' from the exception stack either, since if we
	//	did, %esp would have the wrong value.
	//
	//	In the case of a recursive fault on the exception stack,
	//	note that the world we're pushing now will fit in the
	//	blank word that the kernel reserved for us.
	//
	// Throughout the remaining code, think carefully about what
	// registers are available for intermediate calculations. you
	// may find that you have to re-arrange your code in non-obvious
	// ways as registers become unavailable as scratch space.

	// trap_time_esp (on exception stack) = trap_time_esp - 4, 
	// for inserting trap_time_eip used for 'ret' instruction to return.
	movl	0x30(%esp), %eax
	subl	$0x4, %eax
	movl	%eax, 0x30(%esp)

	// Insert eip on user's stack to serve as 'Return Address'
	movl	0x28(%esp), %ebx
	movl	%ebx, (%eax)

	// Restore the trap-time registers. After you do this, you can
	// no longer modify any general-purpose registers.
	addl	$0x8, %esp
	popal

	// Restore eflags from the stack. After you do this, you can
	// no longer use arithmetic operations or anything else that 
	// modifies eflags.
	addl	$0x4, %esp
	popfl

	// Switch back to the adjusted trap-time stack.
	popl %esp

	// Return to re-execute the instruction that falted.
	ret

