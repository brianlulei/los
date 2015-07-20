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

#define TRAPHANDLER(name, num, priv)											\
.text;																	\
	.globl name;				/* define flobal symbol for 'name' */	\
	.type name, @function;		/* symbol type is function */			\
	.align 2;					/* align function definition */			\
	name:						/* function starts here */				\
	pushl $(num);														\
	jmp _alltraps;														\
.data;																	\
	.long num, name, priv;

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */

#define TRAPHANDLER_NOEC(name, num, priv)	\
.text;										\
	.globl name;							\
	.type name, @function;					\
	.align 2;								\
	name:									\
	pushl $0 ;								\
	pushl $(num);							\
	jmp _alltraps;							\
.data;										\
	.long num, name, priv;


/*
 * Generating entry points for the different traps.
 */

.data
	.global irq_handler_array

irq_handler_array:

TRAPHANDLER_NOEC(irq_handler_divide, T_DIVIDE, 0);
TRAPHANDLER_NOEC(irq_handler_debug, T_DEBUG, 0);
TRAPHANDLER_NOEC(irq_handler_nmi, T_NMI, 0);
TRAPHANDLER_NOEC(irq_handler_brkpt, T_BRKPT, 0);
TRAPHANDLER_NOEC(irq_handler_oflow, T_OFLOW, 0);
TRAPHANDLER_NOEC(irq_handler_bound, T_BOUND, 0);
TRAPHANDLER_NOEC(irq_handler_illop, T_ILLOP, 0);
TRAPHANDLER_NOEC(irq_handler_device, T_DEVICE, 0);
TRAPHANDLER(irq_handler_dblflt, T_DBLFLT, 0);
TRAPHANDLER(irq_handler_tss, T_TSS, 0);
TRAPHANDLER(irq_handler_segnp, T_SEGNP, 0);
TRAPHANDLER(irq_handler_stack, T_STACK, 0);
TRAPHANDLER(irq_handler_gpflt, T_GPFLT, 0);
TRAPHANDLER(irq_handler_pgflt, T_PGFLT, 0);
TRAPHANDLER_NOEC(irq_handler_fperr, T_FPERR, 0);
TRAPHANDLER(irq_handler_align, T_ALIGN, 0);
TRAPHANDLER_NOEC(irq_handler_mchk, T_MCHK, 0);
TRAPHANDLER_NOEC(irq_handler_simderr, T_SIMDERR, 0);
TRAPHANDLER_NOEC(irq_handler_syscall, T_SYSCALL, 3);
TRAPHANDLER_NOEC(irq_handler_default, T_DEFAULT, 0);

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
