#include <include/assert.h>
#include <include/stdarg.h>
#include <include/stdio.h>
#include <kernel/monitor.h>

/*
 * Variable panicstr contains argument to first call to panic;
 * Used as flag to indicate that the kernel has called panic;
 */
const char *panicstr;



void
_panic(const char *file, int line, const char *fmt, ...)
{
	va_list ap;

	if (panicstr)
		goto dead;

	panicstr = fmt;

	__asm __volatile("cli; cld");

	va_start(ap, fmt);
	cprintf("kernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

dead:
	while (1)
	/* do nothing */
		;
}

void
_warn(const char *file, int line, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	cprintf("kernel warning at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);
}
