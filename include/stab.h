#ifndef _STAB_H_
#define _STAB_H_
#include <include/types.h>

// The LOS kernel debugger can understand some debugging information
// in the STABS format.

#define N_FUN	0x24	// fucntion name or text segment variable
#define N_MAIN	0x2a	// name of main routine
#define N_SLINE	0x44	// line number in text segment
#define N_SO	0x64	// path and name of source file
#define N_SOL	0x84	// name of include file


// Entries in the STABS table are formatted as follows.

// The assembler creates two custom sections, a section named .stab
// which contains an array of fixed length structures, one struct
// per stab, and a section named .stabstr containing all the variable
// length strings that are referenced by stabs in the .stab section.

// See "stabs" debug format section 6: Symbol Information in Symbol Tables

typedef struct {
	uint32_t	n_strx;		// index into .stabstr table of name
	uint8_t		n_type;		// stab symbol types
	uint8_t		n_other;	// misc info (usually empty)
	uint16_t	n_desc;		// description field
	uintptr_t	n_value;	// value of symbol
} Stab;

#endif
