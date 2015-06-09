#include <include/string.h>

int strnlen(const char *s, size_t size)
{
	int n;
	for (n = 0; size > 0 && *s != '\0'; s++, size--)
		n++;
	return n;
}

void *
memset(void *dst, int c, size_t n)
{
	char *p = dst;

	while (n-- > 0)
		*p++ = c;

	return dst;
}

void *
memmove(void *dst, const void *src, size_t n)
{
	const char *s = src;
	char *d = dst;

	if (s < d && s + n > d) {
		s += n;
		d += n;
		while (n-- > 0) {
			*--d = *--s;
		}
	} else {
		while (n-- > 0) {
			*d++ = *s++;
		}
	}
	return dst;
}
