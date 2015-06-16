#include <include/stdio.h>
#include <include/x86.h>
#include <include/types.h>
#include <kernel/monitor.h>
#include <kernel/kdebug.h>
#include <include/memlayout.h>
#include <include/string.h>

extern uint32_t	bootstacktop;


struct Command {
	const char *name;
	const char *desc;
	int (*func)(int argc, char **argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{"help", "Display this list of commands", mon_help},
	{"kerninfo", "Display information about the kernel", mon_kerninfo},
	{"backtrace", "Display call stack", mon_backtrace},
};

#define NCOMMANDS (sizeof(commands)/sizeof(struct Command))

int
mon_help(int argc, char ** argv, struct Trapframe *tf)
{
	int i; 
	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];
	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start); 
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
	ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}


int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	int i;
	Eipdebuginfo dbg_info = {};
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

		debuginfo_eip(eip, &dbg_info);

		cprintf("\n%s:%d: %.*s+%d", 
				dbg_info.eip_file,
				dbg_info.eip_line,
				dbg_info.eip_fn_namelen,
				dbg_info.eip_fn_name,
				eip - dbg_info.eip_fn_saddr);
		cprintf("\n");

		// ebp is set to its caller
		ebp = next_ebp;
	}

	return 0;
}

#define WHITESPACE "\t\r\n"
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;
	int first_word = 1;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	
	
	while (*buf) {
		if (strchr(WHITESPACE, *buf)) { // if it is white space
			// set it to 0 and move on
			*buf++ = 0;
			first_word = 1;
		} else {
			// move on
			if (first_word) {
				argv[argc++] = buf;
				first_word = 0;
			}
			buf++;
		}
	}
	argv[argc] = 0;

	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}


void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the LOS kernel monitor\n");
	cprintf("Type 'help' for a list of commands.\n");

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
