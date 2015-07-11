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

// construct linear address from indexes and offset
#define PGADDR(d, t, o)	(void *) (((d) << PDXSHIFT) | ((t) << PTXSHIFT) | (o))

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
#define SEG(type, base, lim, dpl)	(Segdesc)						\
{ ((lim) >> 12) & 0xffff, (base) & 0xffff, ((base) >> 16) & 0xff,	\
  type, 1, dpl, 1, (lim) >> 28, 0, 0, 1, 1,							\
  base >> 24 }	


#define SEG16(type, base, lim, dpl)	(Segdesc)						\
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

// System segment type bits
#define STS_T16A    0x1     // Available 16-bit TSS
#define STS_LDT     0x2     // Local Descriptor Table
#define STS_T16B    0x3     // Busy 16-bit TSS
#define STS_CG16    0x4     // 16-bit Call Gate
#define STS_TG      0x5     // Task Gate / Coum Transmitions
#define STS_IG16    0x6     // 16-bit Interrupt Gate
#define STS_TG16    0x7     // 16-bit Trap Gate
#define STS_T32A    0x9     // Available 32-bit TSS
#define STS_T32B    0xB     // Busy 32-bit TSS
#define STS_CG32    0xC     // 32-bit Call Gate
#define STS_IG32    0xE     // 32-bit Interrupt Gate
#define STS_TG32    0xF     // 32-bit Trap Gate

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


/*
 * Part 3. Traps
 *
 */

#ifndef __ASSEMBLER__

// Task state segment format
typedef struct Taskstate {
	uint32_t	ts_link;	// Old ts selector
	uintptr_t	ts_esp0;	// Stack pointers and segment selectors
	uint16_t	ts_ss0;		// after an increase in privilege level
	uint16_t	ts_padding1;
	uintptr_t	ts_esp1;
	uint16_t	ts_ss1;
	uint16_t	ts_padding2;
	uintptr_t	ts_esp2;
	uint16_t	ts_ss2;
	uint16_t	ts_padding3;
	physaddr_t	ts_cr3;		// Page directory base
	uintptr_t	ts_eip;		// Saved state from last task switch
	uint32_t	ts_eflags;
	uint32_t	ts_eax;		// More saved state (registers)
	uint32_t	ts_ecx;
	uint32_t	ts_edx;
	uint32_t	ts_ebx;
	uintptr_t	ts_esp;
	uintptr_t	ts_ebp;
	uint32_t	ts_esi;
	uint32_t	ts_edi;
	uint16_t	ts_es;
	uint16_t	ts_padding4;
	uint16_t	ts_cs;
	uint16_t	ts_padding5;
	uint16_t	ts_ss;
	uint16_t	ts_padding6;
	uint16_t	ts_ds;
	uint16_t	ts_padding7;
	uint16_t	ts_fs;
	uint16_t	ts_padding8;
	uint16_t	ts_gs;
	uint16_t	ts_padding9;
	uint16_t	ts_ldt;
	uint16_t	ts_padding10;
	uint16_t	ts_t;		// Trap on task switch
	uint16_t	ts_iomb;	// I/O map base address
} Taskstate;

// Gate descriptor for interrupts and traps
typedef struct Gatedesc{
	unsigned	gd_off_15_0	: 16;	// low 16 bits of offset in segment
	unsigned	gd_sel		: 16;	// segment selector
	unsigned	gd_args		: 5;	// # args, 0 for interrupt/trap gates
	unsigned	gd_rsv		: 3;	// reserved (should be zero)
	unsigned	gd_type		: 4;	// type (STS_{TG, IG32, TG32})
	unsigned	gd_s		: 1;	// must be 0 (system)
	unsigned	gd_dpl		: 2;	// descriptor privilege level
	unsigned	gd_p		: 1;	// present
	unsigned	gd_off_31_16: 16;	// high bits of offset in segment
} Gatedesc;




#endif

#endif
