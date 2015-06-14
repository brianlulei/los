#ifndef _STRING_H
#define _STRING_H

#include <include/types.h>

int		strnlen(const char *s, size_t size);
void *	memset(void *dst, int c, size_t len);
void *	memmove(void *dst, const void *src, size_t len);
char *	strfind(const char *s, char c);
#endif
