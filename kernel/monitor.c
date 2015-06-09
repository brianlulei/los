#include <include/stdio.h>
#include <include/x86.h>
#include <include/types.h>
#include <kernel/monitor.h>

extern uint32_t	bootstacktop;

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	int i;
	uintptr_t ebp = (uintptr_t) read_ebp();

	cprintf("BOOT STACKTOP = %p\n", bootstacktop);

	// ebp register has been set to 0x00 before 
    // the 1st function call i386_init. So, it can 
	// terminate when ebp register is 0.
	while (ebp) {
		// caller's ebp 
		uintptr_t next_ebp = *(uint32_t *) ebp;

		// return address of current function call
		uint32_t eip = *((uint32_t *)ebp + 1);

		cprintf("ebp %p eip %p args ", ebp, eip);

		for (i = 0; i < 5; i++) {
			cprintf("%x ", *((uint32_t *)ebp + 2 + i));
		}

		cprintf("\n");

		// ebp is set to its caller
		ebp = next_ebp;
	}

	return 0;
}
