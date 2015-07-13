#include <include/trap.h>
#include <include/mmu.h>
#include <include/memlayout.h>

/*****************************************************************
 * Exceptions / Interrupts
 *****************************************************************/


/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup). You can declare the function with void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */

#define TRAPHANDLER(name, num)											\
	.globl name;				/* define flobal symbol for 'name' */	\
	.type name, @function;		/* symbol type is function */			\
	.align 2;					/* align function definition */			\
	name:						/* function starts here */				\
	pushl $(num);														\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */

#define TRAPHANDLER_NOEC(name, num)	\
	.globl name;						\
	.type name, @function;				\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);						\
	jmp _alltraps

.text

/*
 * Generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(hdler0, T_DIVIDE)
TRAPHANDLER_NOEC(hdler1, T_DEBUG)
TRAPHANDLER_NOEC(hdler2, T_NMI)
TRAPHANDLER_NOEC(hdler3, T_BRKPT)
TRAPHANDLER_NOEC(hdler4, T_OFLOW)
TRAPHANDLER_NOEC(hdler5, T_BOUND)
TRAPHANDLER_NOEC(hdler6, T_ILLOP)
TRAPHANDLER_NOEC(hdler7, T_DEVICE)
TRAPHANDLER(hdler8, T_DBLFLT)
TRAPHANDLER_NOEC(hdler9, 9)
TRAPHANDLER(hdler10, T_TSS)
TRAPHANDLER(hdler11, T_DBLFLT)
TRAPHANDLER(hdler12, T_STACK)
TRAPHANDLER(hdler13, T_GPFLT)
TRAPHANDLER(hdler14, T_PGFLT)
TRAPHANDLER_NOEC(hdler15, 15)
TRAPHANDLER_NOEC(hdler16, T_FPERR)
TRAPHANDLER(hdler17, T_ALIGN)
TRAPHANDLER_NOEC(hdler18, T_MCHK)
TRAPHANDLER_NOEC(hdler19, T_SIMDERR)

/*
 * Code for _alltraps
 */

_alltraps:
	/* Push values to make the stack look like a struct Trapframe*/
	pushl %ds
	pushl %es
	pushal

	movw $GD_KD, %ax
	movw %ax, %ds
	movw %ax, %es

	/* Why need to push %esp after all the pushes? 
	 * Pushl %esp to pass a pointer to the Trapframe as an argument to trap(). */

	pushl %esp

	call trap

.data
	.globl trap_handlers

trap_handlers:
	.long hdler0
	.long hdler1
	.long hdler2
	.long hdler3
	.long hdler4
	.long hdler5
	.long hdler6
	.long hdler7
	.long hdler8
	.long hdler9
	.long hdler10
	.long hdler11
	.long hdler12
	.long hdler13
	.long hdler14
	.long hdler15
	.long hdler16
	.long hdler17
	.long hdler18
	.long hdler19

