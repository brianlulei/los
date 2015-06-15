#include <kernel/console.h>
#include <include/memlayout.h>
#include <include/x86.h>
#include <include/string.h>
#include <include/kbdreg.h>

static uint32_t addr_6845;
static uint16_t	* crt_buf;
static uint16_t	crt_pos;

static void cons_putc(uint16_t c);
static void cons_intr(int (*proc)(void));
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

/***** Keyboard input code *****/

#define NO			0

#define SHIFT		(1<<0)
#define CTL			(1<<1)
#define ALT			(1<<2)
#define CAPSLOCK	(1<<3)
#define NUMLOCK		(1<<4)
#define SCROLLLOCK	(1<<5)
#define E0ESC		(1<<6)


static uint8_t shiftcode[256] = 
{
	[0x1D] = CTL,
	[0x2A] = SHIFT,
	[0x36] = SHIFT,
	[0x38] = ALT,
	[0x9D] = CTL,
	[0xB8] = ALT
};

static uint8_t togglecode[256] = 
{
	[0x3A] = CAPSLOCK,
	[0x45] = NUMLOCK,
	[0x46] = SCROLLLOCK
};

static uint8_t normalmap[256] =
{
    NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
    '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
    'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
    '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
    'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
    NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO, 
    NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
    '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
    [0xC7] = KEY_HOME,        [0x9C] = '\n' /*KP_Enter*/,
    [0xB5] = '/' /*KP_Div*/,      [0xC8] = KEY_UP,
    [0xC9] = KEY_PGUP,        [0xCB] = KEY_LF,
    [0xCD] = KEY_RT,          [0xCF] = KEY_END,
    [0xD0] = KEY_DN,          [0xD1] = KEY_PGDN,
    [0xD2] = KEY_INS,         [0xD3] = KEY_DEL
};

static uint8_t shiftmap[256] =
{
    NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
    '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
    'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
    '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
    NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO, 
    NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
    '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
    [0xC7] = KEY_HOME,        [0x9C] = '\n' /*KP_Enter*/,
    [0xB5] = '/' /*KP_Div*/,      [0xC8] = KEY_UP,
    [0xC9] = KEY_PGUP,        [0xCB] = KEY_LF,
    [0xCD] = KEY_RT,          [0xCF] = KEY_END,
    [0xD0] = KEY_DN,          [0xD1] = KEY_PGDN,
    [0xD2] = KEY_INS,         [0xD3] = KEY_DEL
};

#define C(x) (x - '@')

static uint8_t ctlmap[256] =
{
    NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO, 
    NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO, 
    C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
    C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
    C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO, 
    NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
    C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO, 
    [0x97] = KEY_HOME,
    [0xB5] = C('/'),        [0xC8] = KEY_UP,
    [0xC9] = KEY_PGUP,      [0xCB] = KEY_LF,
    [0xCD] = KEY_RT,        [0xCF] = KEY_END,
    [0xD0] = KEY_DN,        [0xD1] = KEY_PGDN,
    [0xD2] = KEY_INS,       [0xD3] = KEY_DEL
};

static uint8_t *charcode[4] = {
    normalmap,
    shiftmap,
    ctlmap,
    ctlmap
};


/* Get data from the keyboard.
 * Return the ASCII code, if it is a character
 * Return 0, if it is not a character.
 * Return -1, if no data.
 */

static int
kbd_proc_data(void)
{
	int c;
	uint8_t data;
	static uint32_t shift;

	if ((inb(KBSTATP) & KBS_DIB) == 0)
		return -1;

	data = inb(KBDATAP);

	if (data == 0xE0) {
		// E0 escape character
		shift |= E0ESC;
		return 0;
	} else if (data & 0x80) {
		// Key released
		// if E0ESC was received before, is was an escaped key press (e.g. 0xE0, 0x49)
		// otherwise, it was a normal key press (e.g. keypad-9-press)
		data = (shift & E0ESC ? data : data & 0x7F);
		// remove corresponding label 
		shift &= ~(shiftcode[data] | E0ESC);
		return 0; 
	} else if (shift & E0ESC) {
		// Last character was E0 escape; or with 0x80
		// data is or-ed with 0x80 to differenciate with E0 escape and without E0 escape
		data |= 0x80;
		shift &= ~E0ESC;
	}

	shift |= shiftcode[data];
	shift ^= togglecode[data];
	
	c = charcode[shift & (CTL | SHIFT)][data];

	if (shift & CAPSLOCK) {
	 if ('a' <= c && c <= 'z')
            c += 'A' - 'a';
        else if ('A' <= c && c <= 'Z')
            c += 'a' - 'A';
	}

	// Process special keys
	// Ctrl-Alt-Del: reboot
	// TODO
	return c;
}

void
kbd_intr(void)
{
	cons_intr(kbd_proc_data);
}


/* Implement functions defined in stdio.h */

#define CONSBUFSIZE 512

static struct {
	uint8_t		buf[CONSBUFSIZE];
	uint32_t	rpos;	// read position
	uint32_t	wpos;	// write position
} cons;


// called by devide interrupt routines to feed input characters
// into the circular console input buffer.
static void
cons_intr(int (*proc)(void))
{
	int c;

	while (( c = (*proc)()) != -1) {
		if (c == 0)
			continue;
		cons.buf[cons.wpos++] = c;
		if (cons.wpos == CONSBUFSIZE)
			cons.wpos = 0;
	}
}


// return the next input character from the console, or 0 if none waiting
int
cons_getc(void)
{
	int c;

	// poll for any pending input characters,
	// so that this function works wven when interrupts are disabled
	// (e.g., when called from the kernel monitor).

	// serial_intr();
	kbd_intr();

	// grab the next character from the input buffer.
	if (cons.rpos != cons.wpos) {
		c = cons.buf[cons.rpos++];
		if (cons.rpos == CONSBUFSIZE)
			cons.rpos = 0;
		return c;
	}
	return 0;
}

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

int
getchar(void)
{
	int c;
	
	while ((c = cons_getc()) == 0)
		/* do nothing */;
	return c;
}

int iscons(int fdnum)
{
	// used by readline
	return 1;
}
