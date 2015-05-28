#ifndef _MMU_H_
#define _MMU_H_

/*
 * Part 1. Paging data structures and constants.
 *
 */
#define PGSIZE		4096	// bytes mapped by a page
#define PGSHIFT		12		// log2(PGSIZE)

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


// Descriptor Type Bits
#define STA_X	0x08		// Executable (code) segment 
#define STA_E	0x04		// Expand down (data/stack segment)
#define STA_C	0x04		// Conforming code segment (code segment)
#define STA_W	0x02		// Writeable (data/stack segment)
#define STA_R	0x02		// Readable (code segment)
#define STA_A	0x01		// Accessed

// Control Register Flags
#define CR0_PE	0x00000001	// Protection Enable
#define CR0_MP	0x00000002	// Monitor coPorcessor
#define CR0_EM	0x00000004	// Emulation
#define	CR0_TS	0x00000008	// Task Switched
#define CR0_ET	0x00000010	// Extension Type
#define CR0_NE	0x00000020	// Numeric Error
#define CR0_WP	0x00010000	// Write Protect
#define CR0_AM	0x00040000	// Alignment Mask
#define CR0_NW	0x20000000	// Not Writethrough
#define CR0_CD	0x40000000	// Cache Disable
#define CR0_PG	0x80000000	// Paging

#endif
