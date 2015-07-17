#include <include/stdio.h>
#include <include/string.h>

#include <kernel/console.h>
#include <kernel/monitor.h>
#include <kernel/pmap.h>
#include <kernel/env.h>
#include <kernel/trap.h>

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

	ENV_CREATE(hello, ENV_TYPE_USER);

	//
	env_run(&envs[0]);
}
