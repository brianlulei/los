#ifndef	_MEMLAYOUT_H_
#define	_MEMLAYOUT_H_

#ifndef __ASSEMBLER__
#include <include/types.h>
#include <include/mmu.h>
#endif

// All physical memory mapped at this address
#define KERNBASE	0xF0000000

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
