#ifndef _MMU_H_
#define _MMU_H_

/*
 * Part 1. Paging data structures and constants.
 *
 */

#define NPDENTRIES	1024	// page directory entries per page directory
#define NPTENTRIES	1024	// page table entries per page table

#define PGSIZE		4096	// bytes mapped by a page
#define PGSHIFT		12		// log2(PGSIZE)

#define PTSIZE		(PGSIZE*NPTENTRIES) // bytes mapped by a page directory entry (4MB)
#define PTSHIFT		22		// log2(PTSIZE)

#define PTXSHIFT	12		// offset of PTX in a linear address
#define PDXSHIFT	22		// offset of PDX in a linear address


// page number field of physical address
#define PGNUM(la)	(((uintptr_t) (la)) >> PTXSHIFT)

// page directory index
#define PDX(la)		((((uintptr_t) (la)) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(la)		((((uintptr_t) (la)) >> PTXSHIFT) & 0x3FF)




/*
 *
 * Part 2. Segmentation data structures and constants
 *
 */



#ifdef __ASSEMBLER__

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

#else

#include <include/types.h>

// Segment Descriptors
typedef struct {
	unsigned sd_lim_15_0 : 16;	// Low bits of segment limit
	unsigned sd_base_15_0 : 16;	// Low bits of segment base address
	unsigned sd_base_23_16 : 8;	// Middle bits of segment base address
	unsigned sd_type : 4;		// Segment type (see STS_ constants)
	unsigned sd_s : 1;			// 0 = system, 1 = application
	unsigned sd_dpl : 2;		// Descriptor Privilege Level
	unsigned sd_p : 1;			// Present
	unsigned sd_lim_19_16: 4;	// High bits of segment limit
	unsigned sd_avl : 1;		// Unused (available for software use)
	unsigned sd_rsv : 1;		// Reserved
	unsigned sd_db : 1;			// 0 = 16-bit segment, 1 = 32-bit segment
	unsigned sd_g : 1;			// Granularity: limit scaled by 4K when set
	unsigned sd_base_31_24: 8;	// High bits of segment base address
} Segdesc;

// Null segment
#define SEG_NULL	(Segdesc) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

// Segment that is loadable but faults when used
#define SEG_FAULT	(Segdesc) {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0}

// Normal segment
#define SEG(type, base, lim, dpl)	(Segdesc)				\
{ ((lim) >> 12) & 0xffff, (base) & 0xffff, ((base) >> 16) & 0xff,	\
  type, 1, dpl, 1, (lim) >> 28, 0, 0, 1, 1,				\
  base >> 24 }	


#define SEG16(type, base, lim, dpl)	(Segdesc)				\
{ (lim) & 0xffff, (base) & 0xffff, ((base) >> 16) & 0xff,			\
  type, 1, dpl, 1, (lim) >> 16, 0, 0, 1, 0,							\
  (base) >> 24 }

// Pseudo-descriptors used for LGDT, LLDT, and LIDT instructions.
struct Pseudodesc {
	uint16_t	pd_lim;		// Limit
	uint32_t	pd_base;	// Base address
} __attribute__((packed));

typedef struct Pseudodesc Pseudodesc;
#endif 


// Page table/directory entry flags
#define PTE_P	0x001		// Present
#define PTE_W	0x002		// Writeable
#define PTE_U	0x004		// User
#define PTE_PWT	0x008		// Write-Through
#define PTE_PCD	0x010		// Cache-Disable
#define PTE_A	0x020		// Accessed
#define PTE_D	0x040		// Dirty
#define PTE_PS	0x080		// Page Size
#define PTR_G	0x100		// Global

// Adress in page table or page directory entry
#define PTE_ADDR(pte) ((physaddr_t) (pte) & ~0xFFF)

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
