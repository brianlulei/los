#ifndef _KERNEL_TRAP_H_
#define _KERNEL_TRAP_H_

#include <include/mmu.h>
#include <include/trap.h>

/*The kernel's interrupt descriptor table */
extern Gatedesc idt[];
extern Pseudodesc idt_pd;

void trap_init(void);
void trap_init_percpu(void);

#endif
