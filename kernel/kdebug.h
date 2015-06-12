#ifndef _KDEBUG_H_
#define _KDEBUG_H_


// Debug information about a particular instruction pointer
typedef struct {
	const char *	eip_file;		// source code filename for EIP
	int				eip_line;		// source code line number for EIP
	const char *	eip_fn_name;	// function name for EIP
	int				eip_fn_namelen;	// length of function name
	uintptr_t		eip_fn_saddr;	// start address of function for EIP
	int				eip_fn_narg;	// number of function arguments
} Eipdebuginfo;

int debuginfo_eip(uintptr_t eip, Eipdebuginfo * info);

#endif
