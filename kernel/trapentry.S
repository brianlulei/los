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

/* Exceptions (0-31) */
TRAPHANDLER_NOEC(irq_handler_divide, T_DIVIDE, 0);
TRAPHANDLER_NOEC(irq_handler_debug, T_DEBUG, 0);
TRAPHANDLER_NOEC(irq_handler_nmi, T_NMI, 0);
TRAPHANDLER_NOEC(irq_handler_brkpt, T_BRKPT, 0);
TRAPHANDLER_NOEC(irq_handler_oflow, T_OFLOW, 0);
TRAPHANDLER_NOEC(irq_handler_bound, T_BOUND, 0);
TRAPHANDLER_NOEC(irq_handler_illop, T_ILLOP, 0);
TRAPHANDLER_NOEC(irq_handler_device, T_DEVICE, 0);
TRAPHANDLER(irq_handler_dblflt, T_DBLFLT, 0);
// idt 9 reserved
TRAPHANDLER(irq_handler_tss, T_TSS, 0);
TRAPHANDLER(irq_handler_segnp, T_SEGNP, 0);
TRAPHANDLER(irq_handler_stack, T_STACK, 0);
TRAPHANDLER(irq_handler_gpflt, T_GPFLT, 0);
TRAPHANDLER(irq_handler_pgflt, T_PGFLT, 0);
// idt 15 reserved
TRAPHANDLER_NOEC(irq_handler_fperr, T_FPERR, 0);
TRAPHANDLER(irq_handler_align, T_ALIGN, 0);
TRAPHANDLER_NOEC(irq_handler_mchk, T_MCHK, 0);
TRAPHANDLER_NOEC(irq_handler_simderr, T_SIMDERR, 0);

/* System call */
TRAPHANDLER_NOEC(irq_handler_syscall, T_SYSCALL, 3);

/* Hardware IRQ (0-15)*/
TRAPHANDLER_NOEC(idt_irq0, IRQ_OFFSET + 0, 0);
TRAPHANDLER_NOEC(idt_irq1, IRQ_OFFSET + 1, 0);
TRAPHANDLER_NOEC(idt_irq2, IRQ_OFFSET + 2, 0);
TRAPHANDLER_NOEC(idt_irq3, IRQ_OFFSET + 3, 0);
TRAPHANDLER_NOEC(idt_irq4, IRQ_OFFSET + 4, 0);
TRAPHANDLER_NOEC(idt_irq5, IRQ_OFFSET + 5, 0);
TRAPHANDLER_NOEC(idt_irq6, IRQ_OFFSET + 6, 0);
TRAPHANDLER_NOEC(idt_irq7, IRQ_OFFSET + 7, 0);
TRAPHANDLER_NOEC(idt_irq8, IRQ_OFFSET + 8, 0);
TRAPHANDLER_NOEC(idt_irq9, IRQ_OFFSET + 9, 0);
TRAPHANDLER_NOEC(idt_irq10, IRQ_OFFSET + 10, 0);
TRAPHANDLER_NOEC(idt_irq11, IRQ_OFFSET + 11, 0);
TRAPHANDLER_NOEC(idt_irq12, IRQ_OFFSET + 12, 0);
TRAPHANDLER_NOEC(idt_irq13, IRQ_OFFSET + 13, 0);
TRAPHANDLER_NOEC(idt_irq14, IRQ_OFFSET + 14, 0);
TRAPHANDLER_NOEC(idt_irq15, IRQ_OFFSET + 15, 0);

/* Default handler */
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
