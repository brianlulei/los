#ifndef	_MEMLAYOUT_H_
#define	_MEMLAYOUT_H_

#ifndef __ASSEMBLER__
#include <include/types.h>
#include <include/mmu.h>
#endif

#define KERNBASE	0xF0000000

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
