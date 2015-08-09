#include <include/stdio.h>
#include <include/string.h>

#include <kernel/console.h>
#include <kernel/monitor.h>
#include <kernel/pmap.h>
#include <kernel/env.h>
#include <kernel/trap.h>
#include <kernel/cpu.h>
#include <kernel/picirq.h>

static void boot_aps(void);

// Test the stack backtrace function
void
test_backtrace(int x)
{
	cprintf("entering test_backtrace %d\n", x);
	if (x > 0)
		test_backtrace(x - 1);
	else
		mon_backtrace(0, 0, 0);
	cprintf("leaving test_backtrace %d\n", x);
}

void
i386_init(void)
{
	// These two are exposed during linking period
	extern char edata[], end[];

	// Clear the uninitialized global data (BSS) section.
	// This ensures that all static/global variables start with 0
	memset(edata, 0, end - edata);

	// Initialize the console.
	cons_init();

	// Memory management initialization functions
	mem_init();

	// User environment initialization functions
	env_init();
	
	// TSS and interrupt initialization functions
	trap_init();

	// Multiprocessor initalization functions
	mp_init();
	lapic_init();

	pic_init();

	// Acquire the big kernel lock before waking up APs

	// Starting non-boot CPUs
	boot_aps();

	ENV_CREATE(user_buggyhello, ENV_TYPE_USER);

	//
	env_run(&envs[0]);
}

/*************************************************************************
 * While boot_aps is booting a given CPU, it communicates the per-core stack
 * pointer that should be loaded by mpentry.S to that CPU in this variable.
 *************************************************************************/
void *mpentry_kstack;

/* Start the non-boot (AP) processors */
static void
boot_aps(void)
{
	extern unsigned char mpentry_start[], mpentry_end[];
	void *code;
	CpuInfo *c;

	// Write entry code to unused memory at MPENTRY_PADDR
	code = KADDR(MPENTRY_PADDR);
	memmove(code, mpentry_start, mpentry_end - mpentry_start);

	// Boot each AP one at a time
	for (c = cpus; c < cpus + ncpu; c++) {
		if (c == cpus + cpunum())	// We've started already.
			continue;

		// Tell mpentry.S what stack to use
		mpentry_kstack = percpu_kstacks[c - cpus] + KSTKSIZE;

		// Start the CPU at mentry_start
		lapic_startap(c->cpu_id, PADDR(code));

		// Wait for the CPU to finish some basic setup in mp_main()
		while (c->cpu_status != CPU_STARTED)
			;
	}
}

/* Setup code for APs */
void
mp_main(void)
{
	// We are in high EIP now, safe to switch to kern_pgdir
	lcr3(PADDR(kern_pgdir));
	cprintf("SMP: CPU %d starting\n", cpunum());

	lapic_init();
	env_init_percpu();
	trap_init_percpu();
	xchg(&thiscpu->cpu_status, CPU_STARTED);	// tell boot_aps() we're up

	for (;;);	
}
