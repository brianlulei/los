#ifndef _STDIO_H_
#define _STDIO_H_

#include <include/stdarg.h>

#ifndef NULL
#define NULL	((void *) 0)
#endif


/* kernel/printf.c */
int		cprintf(const char *fmt, ...);
int		vcprintf(const char *fmt, va_list);

/* lib/printfmt.c */
void	vprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt, va_list);		
#endif
