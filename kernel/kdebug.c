#include <include/stab.h>
#include <kernel/kdebug.h>
#include <include/stdio.h>
#include <include/assert.h>
#include <include/memlayout.h>
#include <include/string.h>

extern const Stab __STAB_BEGIN__[];		// begin of stabs table
extern const Stab __STAB_END__[];		// end of stabs table
extern const char __STABSTR_BEGIN__[];	// begin of stabstr table
extern const char __STABSTR_END__[];	// end of stabstr table


// Some stab types are arranged in increasing order by instruction
// address. For example, N_FUN stabs (stab entries with n_type ==
// N_FUN) which mark functions, and N_SO stabs, which mark source files.
//
// Given an instruction address, this function finds the single stab entry
// of type 'type' that contains that address.
//
// The search takes place within the rage [*region_left, *region_right].
// Thus, to search an entire set of N stabs, you might do:
//		left = 0;
//		right = N - 1;		// rightmost stab
//		stab_binsearch(stabs, &left, &right, type, addr);
//
// The search modifies *region_left and *region_right to bracket the
// 'addr'. *region_left points to the matching stab that contains
// 'addr', and *region_right points just before the next stab. If
// *region_left > *region_right, then 'addr' is not contained in any
// matching stab.

static void
stab_binsearch(const Stab *stab, int *region_left, int *region_right,
			   int type, uintptr_t addr)
{
	int l = *region_left, r = *region_right;
	int any_matches = 0;

	while (l <= r) {
		int middle_value = (l + r)/2;
		int mid =  middle_value;

		while (mid >= l && stab[mid].n_type != type) {
			//cprintf("loop -- 1, l = %d, mid = %d\n", l, mid);
			mid--;
		}

		if (mid < l) {	// no match in [l, mid]
			l = middle_value + 1;
			continue;
		}

		// match type 'type', now do bin-search
		any_matches = 1;
		if (stab[mid].n_value < addr) {
			// find a match, so change region_left
			*region_left = mid;
			l = mid + 1;
		} else if (stab[mid].n_value > addr) {
			// find a match, so change region_right
			*region_right = mid - 1;
			r = mid - 1;
		} else {
			// exact match for 'addr', but continue loop to
			// find *region_right.
			*region_left = mid;
			l = mid;
			addr ++; // why ++ addr?
		}
	}
	if (!any_matches) // couldn't find any stab with the 'type'
		*region_right = *region_left - 1;
}


int
debuginfo_eip(uintptr_t addr, Eipdebuginfo * info)
{
	const Stab *stab_start, *stab_end;
	const char *stabstr_start, *stabstr_end;
	int lfile, rfile, lfun, rfun, lline, rline;
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
		panic("User address");
	}

	// String tabl validity check
	if (stabstr_end <= stabstr_start || stabstr_end[-1] != 0)
		return -1;

	// Now we need to find the corrct file, function, line 
	// information for the 'eip'.

	/*
	const Stab *temp = stab_start;
	for ( ;temp <= stab_end; temp++) {
		cprintf("type: %d, ", temp->n_type);
		cprintf("value: %p ", temp->n_value);
		cprintf("\n");
		temp++;
	}*/

	lfile = 0;
	rfile = stab_end - stab_start - 1; // since start from 0
	stab_binsearch(stab_start, &lfile, &rfile, N_SO, addr);
	if (lfile == 0)
		return -1;

	lfun = lfile;
	rfun = rfile;
	stab_binsearch(stab_start, &lfun, &rfun, N_FUN, addr);

	if (lfun <= rfun) {
		// stab_start[lfun] points to the function name
		// in the string table
		if (stab_start[lfun].n_strx < stabstr_end - stabstr_start) 
			info->eip_fn_name = stabstr_start + stab_start[lfun].n_strx;
		info->eip_fn_saddr = stab_start[lfun].n_value;
		addr -= info->eip_fn_saddr;

		// Search whthin the function definition for the line number.
		lline = lfun;
		rline = rfun;
	} else {
		// Cound't find function stab! Maybe we're in an assembly file.
		// Search the whole file for line number.
		info->eip_fn_saddr = addr;
		lline = lfile;
		rline = rfile;
	}

	// Ignore stuff adter ':'
	info->eip_fn_namelen = strfind(info->eip_fn_name, ':') - info->eip_fn_name;

	// Search within [lline, rline] for the line number stab.
	// If found, set info->eip_line to the right number.
	// If not found, return -1.
	stab_binsearch(stab_start, &lline, &rline, N_SLINE, addr);
	if (lline <= rline) // found
		info->eip_line = stab_start[lline].n_desc;
	else
		return -1;

	// We can't user the 'lfile' stab because included functions
	// can interpolate code from a different file
	// such included source files use the N_SOL stab type

	while (lline >= lfile
			&& stab_start[lline].n_type != N_SOL
			&& (stab_start[lline].n_type != N_SO || !stab_start[lline].n_value))
		lline--;
	if (lline >= lfile && stab_start[lline].n_strx < stabstr_end - stabstr_start)
		info->eip_file = stabstr_start + stab_start[lline].n_strx;

	return 0;
}
