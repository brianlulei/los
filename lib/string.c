#include <include/string.h>


int
strlen(const char *s)
{
	int n;

	for (n = 0; *s != '\0'; s++)
		n++;
	return n;
}

int
strnlen(const char *s, size_t size)
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

int
memcmp(const void *v1, const void *v2, size_t n)
{
	const uint8_t *s1 = (const uint8_t *)v1;
	const uint8_t *s2 = (const uint8_t *)v2;

	while (n-- > 0) {
		if (*s1 != *s2)
			return (int) *s1 - (int) *s2;
		s1++, s2++;
	}
	return 0;
}

int
strcmp(const char *s, const char *t)
{
	while (*s && *t && *s == *t) {
		s++;
		t++;
	}
	return (int) ((unsigned int)*s - (unsigned int)*t);
}

char *
strchr(const char *s, char c)
{
	for (; *s; s++) {
		if (*s == c)
			return (char *) s;
	}
	return 0;
}

// Return a pointer to the first occurrence of 'c' in 's',
// or a pointer to the string-ending null character is the string has no 'c'
char *
strfind(const char *s, char c)
{
	const char *temp = s;
	while (*temp) {
		if (*temp == c)
			return (char *)temp;
		temp ++;
	}
	return NULL;
}
