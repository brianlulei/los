#include <include/string.h>

void *
memset(void *dst, int c, size_t n)
{
	char *p = dst;

	while (n-- > 0)
		*p++ = c;

	return dst;
}
