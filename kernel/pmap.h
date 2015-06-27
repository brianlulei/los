#ifndef _PMAP_H_
#define _PMAP_H_
#include <include/types.h>
#include <include/assert.h>
#include <include/memlayout.h>

extern char bootstacktop[], bootstack[];
extern size_t		npages;
extern PageInfo *	pages;


void		mem_init(void);
void		page_init(void);

PageInfo *	page_alloc(int alloc_flag);
void		page_free(PageInfo *);

int			page_insert(pde_t *pgdir, PageInfo *pp,  void *va, int perm);
void		page_remove(pde_t *pgdir, void *va);

PageInfo *	page_lookup(pde_t *pgdir, void *va, pte_t **pte_store);
void		tlb_invalidate(pde_t *pgdir, void *va);
pte_t *		pgdir_walk(pde_t *pgdir, const void *va, int create);

/* This macro takes a kernel virtual address (above KERNBASE)
 * and returns the corresponding physical address.
 */
#define PADDR(kva) _paddr(__FILE__, __LINE__, kva)

static inline physaddr_t
_paddr(const char *file, int line, void *kva)
{
	if ((uint32_t)kva < KERNBASE)
		_panic(file, line, "PADDR called with invalid kva %p", kva);
	return (physaddr_t)kva - KERNBASE;
}

/* This macro takes a physical address and returns the correspoinding kernel
 * virtual address. It panics if pass an invalid physical address.
 */
#define KADDR(pa) _kaddr(__FILE__, __LINE__, pa)

static inline void *
_kaddr(const char *file, int line, physaddr_t pa)
{
	if (PGNUM(pa) >= npages)
		_panic(file, line, "KADDR called with invalid pa %p", pa);
	return (void *)(pa + KERNBASE);
}

static inline physaddr_t
page2pa(PageInfo *pp)
{
	return ((pp - pages) << PGSHIFT);
}

static inline PageInfo *
pa2page(physaddr_t pa)
{
	if (PGNUM(pa) >= npages)
		panic ("pa2page called with invalid pa.");
	return &pages[PGNUM(pa)];
}

static inline void *
page2kva(PageInfo *pp)
{
	return KADDR(page2pa(pp));
}

enum {
	ALLOC_ZERO = 1<<0,	
};
#endif
