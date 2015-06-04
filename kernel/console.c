#include <kernel/console.h>
#include <include/memlayout.h>
#include <include/x86.h>
#include <include/string.h>

static uint32_t addr_6845;
static uint16_t	* crt_buf;
static uint16_t	crt_pos;

static void cons_putc(uint16_t c);

/* Text-mode CGA/VGA display output */

/* Typically, the Video Memory is mapped as the following:

0xA0000 - 0xBFFFF Video Memory used for graphics modes
	* 0xB0000 - 0xB7777 Monochrome Text mode
	* 0xB8000 - 0xBFFFF Color text mode and CGA compatible graphics modes

Port Mapping

The CRT Controller uses a single Data Register which is mapped to port 0x3D5. 
The CRT Controller uses a special register - an Index Register, to determin 
the type of data in the Data Register is. So, in order to give data to the CRT
Controller, we have too write two values. One to the Index Register (Containing
the type of data we are writing), and one to the Data Register (Containing the
data).

The Index Register is mapped to ports *0x3D5* or *0x3B5*.
The Data Register is mapped to ports *0x3D4* or *0x3B4*.

There are more registers then these two (Such as the Misc. Output Register),
but we will focus on these two for now. */

static void
cga_init(void)
{
	volatile uint16_t *cp;
	uint16_t was;
	uint32_t pos;

	cp = (uint16_t *) (KERNBASE + CGA_BUFFER);
	was = *cp;
	*cp = (uint16_t) 0xA55A;
	if (*cp != 0xA55A) {
		cp = (uint16_t *) (KERNBASE + MONO_BUFFER);
		addr_6845 = MONO_BASE;
	} else {
		*cp = was;
		addr_6845 = CGA_BASE;
	}

	/* Extract cursor location */
	/* 0xE	Cursor Location High
	   0xF	Cursor Location Low */
	outb(addr_6845, 0x0E);
	pos = inb(addr_6845 + 1) << 8;

	outb(addr_6845, 0x0F);
	pos |= inb(addr_6845 + 1);

	crt_buf = (uint16_t *) cp;
	crt_pos = pos;
}

static void
cga_putc(uint16_t c)
{
	//
	if (!(c & ~0xFF))
		c |= 0x0700;

	switch (c & 0xFF) {
		case '\b':
			if (crt_pos > 0) {
				crt_pos --;
				crt_buf[crt_pos] = (c & ~0xFF) | ' ';
			}
			break;
		case '\n':
			crt_pos += CRT_COLS;
			/* fall through */
		case '\r':
			crt_pos -= (crt_pos % CRT_COLS);
			break;
		case '\t':
			cons_putc(' ');
			cons_putc(' ');
			cons_putc(' ');
			cons_putc(' ');
			break;
		default:
			crt_buf[crt_pos++] = c;
			break;
	}

	// move up by 1 line
	if (crt_pos >= CRT_SIZE) {
		int i;

		memmove(crt_buf, crt_buf + CRT_COLS, (CRT_SIZE - CRT_COLS) * sizeof(uint16_t));
		for (i = CRT_SIZE - CRT_COLS; i < CRT_SIZE; i++) {
			crt_buf[i] = 0x0700 | ' ';
		}
		crt_pos -= CRT_COLS;
	}
	/* move the cursor*/
	outb(addr_6845, 0x0E);
	outb(addr_6845 + 1, crt_pos >> 8);
	outb(addr_6845, 0x0F);
	outb(addr_6845 + 1, crt_pos);
}


// initialize the console devices
void
cons_init(void)
{
	cga_init();
}

/* Implement functions defined in stdio.h */
static void
cons_putc(uint16_t c)
{
	cga_putc(c);
}

void
cputchar(int c)
{
	cons_putc(c);
}
