#include <include/memlayout.h>
#include <kernel/pmap.h>
#include <kernel/kclock.h>
#include <include/stdio.h>
#include <include/assert.h>

// These variables are set by i386_detect_memory()
size_t			npages;			// Amount of physical memory (in pages)
static size_t	npages_basemem;	// Amount of base memory (in pages)


/*******************************************
 * Detect machine's physical memory setup. *
 *******************************************/
static int
nvram_read(int r)
{
	// Read both low and high bytes to compute the whole value
	return mc146818_read(r) | (mc146818_read(r + 1) << 8);
}


static void
i386_detect_memory(void)
{
	size_t npages_extmem;

	// Use CMOS calls to measure available base & extended memory.
	npages_basemem = (nvram_read(NVRAM_BASELO) * 1024) / PGSIZE;
	npages_extmem = (nvram_read(NVRAM_EXTLO) * 1024) / PGSIZE;

	if (npages_extmem)
		npages = (EXTPHYSMEM / PGSIZE) + npages_extmem;
	else
		npages = npages_basemem;


	cprintf("Physical memory: %uK available, base = %uK, extended = %uK\n",
			npages * PGSIZE / 1024, 
			npages_basemem * PGSIZE / 1024,
			npages_extmem * PGSIZE / 1024);

}

/* Set up a two level page table:
 *
 */
void
mem_init(void)
{
	// Find out how much memory the machine has (npages & npages_basemem).
	i386_detect_memory();
	panic("mem_init: This function is not finished\n");
}
