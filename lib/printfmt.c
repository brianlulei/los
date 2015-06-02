#include <include/stdio.h>

void
vprintfmt(void (*putch)(int, void *), void *putdat, const char *fmt, va_list ap)
{
	register const char *p;
	register int ch, err;
	char padc;
	int precision, lflag, width;

	while (1) {

		// Just printout characters without escape sequence
		while ((ch = *(unsigned char *) fmt++) != '%') {
			if (ch == '\0')
				return;
			putch(ch, putdat);
		}


	reswitch:
		switch (ch = *(unsigned char *) fmt++) {
		// unrecognized escape sequence - just print it literally
		default:
			putch('%', putdat);
			for (fmt--; fmt[-1] != '%'; fmt--)
				/* do nothing*/;
			break;
		}
	}
}
