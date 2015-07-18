#include <include/stdarg.h>
#include <include/stdio.h>
#include <include/lib.h>
#include <include/types.h>

// Collect up to 256 characters into a buffer
// and perform ONE system call to print all of them
// in order to make the lines output to the console atomic
// and prevent interrupts from causing context switches
// in the middle of a console output line and suck.

typedef struct {
	int idx;		// current buffer index
	int cnt;		// total bytes printed so far
	char buf[256];
} printbuf;

static void
putch(int ch, printbuf *b)
{
	b->buf[b->idx++] = ch;
	if (b->idx == 256 - 1) {
		sys_cputs(b->buf, b->idx);
		b->idx = 0;
	}
	b->cnt++;
}

int
vcprintf(const char *fmt, va_list ap)
{
	printbuf b;

	b.idx = 0;
	b.cnt = 0;
	vprintfmt((void *)putch, &b, fmt, ap);
	sys_cputs(b.buf, b.idx);

	return b.cnt;
}

int
cprintf(const char *fmt, ...)
{
	va_list ap;
	int cnt;

	va_start(ap, fmt);
	cnt = vcprintf(fmt, ap);
	va_end(ap);

	return cnt;
}
