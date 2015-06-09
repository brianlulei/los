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
		// Left justified: flag to pad on the right
		case '-':
			padc = '-';
			goto reswitch;

		// flag to pad with 0's instead of spaces
		case '0':
			padc = '0';
			goto reswitch;

		// Minimum characters to print
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			for (precision = 0; ; ++fmt) {
				precision = precision * 10 + ch - '0';
				ch = *fmt;
				if (ch < '0' || ch > '9')
					break;
			}
			goto process_precision;

		case '*':
			precision = va_arg(ap, int);
			goto process_precision;

		// long flag
		case 'l':
			lflag++;
			goto reswitch;

		// character
		case 'c':
			putch(var_arg(ap, int), putdat);
			break;

		// string
		case 's':
			if ((p = va_arg(ap, char *)) == NULL)
				p = "(null)";

			// If left justified
			if (width > 0 && padc != '-')
				for (width -= strnlen(p, precision); width > 0; width--)
					putch(padc, putdat);

				for (; (ch = *p++) != '\0'; width--)
					putch(ch, pudat);

				for (; width > 0; width--)
					putch(' ', putdat);
				break;

				 	
		process_precision:
			if (width < 0) {
				width = precisoin;
				precision = -1;
			}
			goto reswitch;

		default:
			putch('%', putdat);
			for (fmt--; fmt[-1] != '%'; fmt--)
				/* do nothing*/;
			break;
		}
	}
}
