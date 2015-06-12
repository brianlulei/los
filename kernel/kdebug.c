#include <include/stab.h>
#include <kernel/kdebug.h>
#include <include/stdio.h>

extern const Stab __STAB_BEGIN__[];		// begin of stabs table
extern const Stab __STAB_END__[];		// end of stabs table
extern const char __STABSTR_BEGIN__[];	// begin of stabstr table
extern const char __STABSTR_END__[];	// end of stabstr table

int
debuginfo_eip(uintptr_t addr, Eipdebuginfo * info)
{
	const Stab *stab_start, *stab_end;
	const char *stabstr_start, *stabstr_end;

	// Initialize the *info
	info->eip_file = "<unknown>";
	info->eip_line = 0;
	info->eip_fn_name = "<unknown>";
	info->eip_fn_namelen = 0;
	info->eip_fn_saddr = addr;

	if (addr >= ULIM) {
		stab_start = __STAB_BEGIN__;
		stab_end = __STAB_END__;
		stabstr_start = __STABSTR_BEGIN__;
		stabstr_end = __STABSTR_END__;
	} else {
		cprintf("should not be here!");
	}
}
