#ifndef _ERROR_H_
#define _ERROR_H_

enum {
	// Kernel error code -- keep in sync with list in lib/printfmt.c
	E_UNSPECIFIED	= 1, // Unspecified or unknown problem
	E_BAD_ENV		= 2, // Environment doesn't exist or otherwise

	E_INVAL			= 3, // Invalid parameter
	E_NO_MEM		= 4, // Request failed due to memory shortage
	E_NO_FREE_ENV	= 5, //	Attempt to create a new environement beyond

	E_FAULT			= 6, // Memory fault
	E_NO_SYS		= 7, // Unimplemented system call

	E_IPC_NOT_RECV	= 8, // Attempt to send to env that is not receiving

	MAXERROR
};
#endif
