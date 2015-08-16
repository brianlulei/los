#ifndef	_MEMLAYOUT_H_
#define	_MEMLAYOUT_H_

#ifndef __ASSEMBLER__
#include <include/types.h>
#include <include/mmu.h>
#endif

// All physical memory mapped at this address
#define KERNBASE	0xF0000000

// Global descriptor numbers
#define GD_KT		0x08	// kernel text
#define GD_KD		0x10	// kernel data
#define GD_UT		0x18	// user text
#define GD_UD		0x20	// user data
#define GD_TSS0		0x28	// Task segment selector for CPU 0



// At IOPHYSMEM (640K) there is a 384K hole for I/O. From the kernel,
// IOPHYSMEM can be addressed at KERNBASE + IOPHYSMEM. The hole ends
// at physical address EXTPHYSMEM
#define IOPHYSMEM	0x0A0000
#define EXTPHYSMEM	0x100000

// Kernel stack.
#define KSTACKTOP	KERNBASE
#define KSTKSIZE	(8*PGSIZE)		// size of a kernel stack
#define	KSTKGAP		(8*PGSIZE)		// size of a kernel stack guards

// Memory-mapped IO.
#define MMIOLIM		(KSTACKTOP - PTSIZE)
#define MMIOBASE	(MMIOLIM - PTSIZE)

#define ULIM		MMIOBASE

/*
 * User read-only mappings! Anything below here til UTOP are readonly to user.
 * They are global pages mapped in at env allocation time
 */

// User read-only vritual page table
#define UVPT		(ULIM - PTSIZE)

// Read-only copies of the PageInfo structures
#define UPAGES		(UVPT - PTSIZE)

// Read-only copies of the global env structures
#define UENVS		(UPAGES - PTSIZE)

/*
 * Top of user VM. User can manipulate VA from UTOP-1 and down!
 */

// Top of user-accessible VM
#define UTOP		UENVS

// Top of one-page user exception stack
#define UXSTACKTOP	UTOP

// Next page left invalid to guard against exception stack overflow; then:
// Top of normal user stack
#define USTACKTOP	(UTOP - 2*PGSIZE)

// Where user programs generally begin
#define UTEXT		(2*PTSIZE)

// Used for temporary page mappings.  Typed 'void*' for convenience
#define UTEMP		((void*) PTSIZE)

// Physical address of startup code for non-boot CPUs (APs)
#define MPENTRY_PADDR   0x7000



#ifndef __ASSEMBLER__

typedef uint32_t	pte_t;
typedef uint32_t	pde_t;


/*
 * Page descriptor structures, mapped at UPAGES.
 * Read/write to the kernel. read-only to user programs.
 *
 * Each struct PageInfo stores metadata for one physical page.
 * It is NOT the physical page itself, but there is a one-to-one
 * correspondence between physical pages and struct PageInfo's
 * You can map a struct PageInfo * to the corresponding pysical
 * address with page2pa() in kernel/pmap.h
 */

struct PageInfo{
	// Next page on the free list.
	struct PageInfo * pp_link;

	// pp_ref is the count of pointers (usually in page table entries)
	// to this page, for pages allocated using page_alloc.
	// Pages allocated at boot time using pmap.c's boot_alloc()
	// do not have valid reference count fields.

	uint16_t pp_ref;
};

typedef struct PageInfo PageInfo;
#endif
#endif
