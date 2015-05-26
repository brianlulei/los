#ifndef _MMU_H_
#define _MMU_H_


/*
 *	Macros to build GDT entries in assembly.
 */
#define SEG_NULL			\
	.word 0, 0;				\
	.byte 0, 0, 0, 0

// limit uses 4K bytes for 1 unit (>>12)
#define SEG(type, base, lim)							\
	.word (((lim) >> 12) & 0xffff), ((base) & 0xffff);	\
	.byte (((base) >> 16) & 0xff), (0x90 | (type)),		\
		  (0xC0 | (((lim) >> 28) & 0x0f)), (((base) >> 24) & 0xff)


// descriptor type bits
#define STA_X	0x08		// Executable (code) segment 
#define STA_E	0x04		// Expand down (data/stack segment)
#define STA_C	0x04		// Conforming code segment (code segment)
#define STA_W	0x02		// Writeable (data/stack segment)
#define STA_R	0x02		// Readable (code segment)
#define STA_A	0x01		// Accessed

#endif
