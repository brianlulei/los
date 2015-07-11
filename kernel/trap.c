#include <include/mmu.h>
#include <include/memlayout.h>
#include <include/x86.h>

#include <kernel/trap.h>
#include <kernel/env.h>

static Taskstate ts;

// Interrupt descriptor table. 
Gatedesc idt[265] = { {0} };
Pseudodesc idt_pd = {
	sizeof(idt) - 1, (uint32_t) idt
};


void
trap_init(void)
{

	// Per-CPU setup
	trap_init_percpu();
}

// Initialize and load the per-CPU TSS and IDT
void
trap_init_percpu(void)
{
	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	ts.ts_esp0 = KSTACKTOP;
	ts.ts_ss0 = GD_KD;

	// Initialize the TSS slot of the gdt.
	gdt[GD_TSS0 >> 3] = SEG16(STS_T32A, (uint32_t) (&ts),
							  sizeof(Taskstate) - 1, 0);
	gdt[GD_TSS0 >> 3].sd_s = 0;

	// Load the TSS selector
	ltr(GD_TSS0);
}
