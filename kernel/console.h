#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <include/types.h>

#define MONO_BASE		0x3B4
#define MONO_BUFFER		0xB0000
#define CGA_BASE		0x3D4
#define CGA_BUFFER		0xB8000

#define CRT_ROWS		25
#define CRT_COLS		80
#define CRT_SIZE		(CRT_ROWS * CRT_COLS)

void	cons_init(void);
int		cons_getc(void);

void	kbd_intr(void);
void	serial_intr(void);

#endif
