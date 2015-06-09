#include <include/stdio.h>
#include <include/stdarg.h>
#include <include/types.h>
#include <include/string.h>

static void
printnum (void (*putch)(int, void*), void * putdat, unsigned long long num,
		  unsigned base, int width, int padc)
{
	if (num >= base) {
		printnum(putch, putdat, num / base, base, width - 1, padc);
	} else {
		while (--width > 0)
			putch(padc, putdat);
	}
	putch("0123456789abcdef"[num % base], putdat);
}	

static unsigned long long
getuint(va_list *ap, int lflag)
{
	if (lflag >= 2)
		return va_arg(*ap, unsigned long long);
	else if (lflag)
		return va_arg(*ap, unsigned long);
	else
		return va_arg(*ap, unsigned int);
}

static long long
getint(va_list *ap, int lflag)
{
	if (lflag >= 2)
		return va_arg(*ap, long long);
	else if (lflag)
		return va_arg(*ap, long);
	else
		return va_arg(*ap, int);
}


void
vprintfmt(void (*putch)(int, void *), void *putdat, const char *fmt, va_list ap)
{
	register const char *p;
	register int ch, err;
	unsigned long long num;
	char padc;
	int base, precision, lflag, width, altflag;

	while (1) {

		// Just printout characters without escape sequence
		while ((ch = *(unsigned char *) fmt++) != '%') {
			if (ch == '\0')
				return;
			putch(ch, putdat);
		}

		padc = ' ';
		width = -1;
		precision = -1;
		lflag = 0;
		altflag = 0;
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
		
		process_precision:
			if (width < 0) {
				width = precision;
				precision = -1;
			}
			goto reswitch;

		// long flag
		case 'l':
			lflag++;
			goto reswitch;

		// character
		case 'c':
			putch(va_arg(ap, int), putdat);
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
				putch(ch, putdat);

			for (; width > 0; width--)
				putch(' ', putdat);
			break;

		// (signed) decimal
		case 'd':
			num = getint(&ap, lflag);
			base = 10;
			goto number;

		// (unsigned) decimal
		case 'u':
			num = getuint(&ap, lflag);
			base = 10;
			goto number;
				 	
		// (unsigned) octal
		case 'o':
			// Replace this with your code
			num = getuint(&ap, lflag);
			base = 8;
			goto number;

		// pointer
		case 'P':
			putch('0', putdat);
			putch('x', putdat);
			num = (unsigned long long)
					(uintptr_t) va_arg(ap, void *);
			base = 16;
			goto number;

		// (unsigned) hexadecimal
		case 'x':
			num = getuint(&ap, lflag);
			base = 16;

		number:
			printnum(putch, putdat, num, base, width, padc);
			break;

		case '%':
			putch(ch, putdat);
			break;

		default:
			putch('%', putdat);
			for (fmt--; fmt[-1] != '%'; fmt--)
				/* do nothing*/;
			break;
		}
	}
}
