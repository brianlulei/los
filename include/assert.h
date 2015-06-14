#ifndef _ASSERT_H_
#define _ASSERT_H_
#include <include/stdio.h>

void _warn(const char *, int, const char *, ...);
void _panic(const char *, int, const char *, ...) __attribute__((noreturn));

#define warn(...)	_warn(__FILE__, __LINE__, __VA_ARGS__)
#define panic(...)	_panic(__FILE__, __LINE__, __VA_ARGS__)

#endif
