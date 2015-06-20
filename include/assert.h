#ifndef _ASSERT_H_
#define _ASSERT_H_
#include <include/stdio.h>

void _warn(const char *, int, const char *, ...);
void _panic(const char *, int, const char *, ...) __attribute__((noreturn));

#define warn(...)	_warn(__FILE__, __LINE__, __VA_ARGS__)
#define panic(...)	_panic(__FILE__, __LINE__, __VA_ARGS__)

// This is the normal way to define multi-statement macro in C.
// The '#' operator is called stringizing operator which convert
//any argument in the macro function in the string.

#define assert(x)		\
	do {if (!(x)) panic("assertion failed: %s", #x);} while(0)

#endif
