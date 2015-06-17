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

#endif
#endif
