#ifndef	_MEMLAYOUT_H_
#define	_MEMLAYOUT_H_

#define KERNBASE	0xF0000000

// Kernel stack.
#define KSTACKTOP	KERNBASE
#define KSTKSIZE	(8*PGSIZE)		// size of a kernel stack
#define	KSTKGAP		(8*PGSIZE)		// size of a kernel stack guards

#endif
