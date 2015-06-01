#include <include/stdio.h>
#include <include/string.h>

#include <kernel/console.h>

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
	cprintf("Finally we see something from screen!");
}
