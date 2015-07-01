#ifndef _TRAP_H_
#define _TRAP_H_

#ifndef __ASSEMBLER__

#include <include/types.h>

struct PushRegs {
	/* registers are pushed by pusha */
	uint32_t	reg_edi;
	uint32_t	reg_esi;
	uint32_t	reg_ebp;
	uint32_t	reg_oesp;
	uint32_t	reg_ebx;
	uint32_t	reg_edx;
	uint32_t	reg_ecx;
	uint32_t	reg_eax;
} __attribute__((packed));

struct Trapframe {
	struct PushRegs	tf_regs;
	uint16_t		tf_es;
	uint16_t		tf_padding1;
	uint16_t		tf_ds;
	uint16_t		tf_padding2;
	uint32_t		tf_trapno;
	// below here defined by x86 hardware
	uint32_t		tf_err;
	uintptr_t		tf_eip;
	uint16_t		tf_cs;
	uint16_t		tf_padding3;
	uint32_t		tf_eflags;
	// below here only when crossing rings, such as from user to kernel
	uintptr_t		tf_esp;
	uint16_t		tf_ss;
	uint16_t		tf_padding4;
} __attribute__((packed));

#endif

#endif
