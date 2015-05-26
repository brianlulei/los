/*
 *  main.c
 *  Copyright (C) 2015  Lei Lu (lulei.wm@gmail.com)
 */

#include <include/elf.h>
#include <include/x86.h>
#include <include/types.h>

#define SECTSIZE	512
#define ELFHDR		((Elf32_Ehdr *) 0x10000)	// Extended memory

void
bootmain()
{
	Elf32_Phdr *ph, *eph;

	// read 1st page off disk
	readseg((uint32_t) ELFHDR, SECTSIZE*8, 0);

	// is this a valid ELF?
	if (ELFHDR->e_magic != ELF_MAGIC)
		goto bad;

	// load each program segment (ignores ph flags)
	ph = (Elf32_Phdr *) ((uint8_t *) ELFHDR + ELFHDR->e_phoff);
	eph = ph + ELFHDR->e_phnum;
	for (; ph < eph; ph++) {
		// p_pa is the load address of this segment
		// (as well as the physical address)
		readseg(ph->p_paddr, ph->p_memsz, ph->p_offset);
	}

	// call the entry point from the ELF header
	// note: does not return !
	((void (*)(void)) (ELFHDR->e_entry))();
bad:
	outw(0x8A00, 0x8A00);
	outw(0x8A00, 0x8E00);
	while (1)
		/* do nothing */;
}

/* 
 * Read 'count' bytes at 'offset' from kernel into physical address 'pa'
 */
void
readseg(uint32_t pa, uint32_t count, uint32_t offset)
{
	uint32_t end_pa = pa + count;
	
	// round down to sector boundary
	end_pa &= ~(SECTSIZE - 1);

	// translate from bytes to sectors, and kernel start from sector 1
	offset = (offset / SECTSIZE) + 1;

	while (pa < end_pa) {
		readsect((uint8_t *) pa, offset);
		pa += SECTSIZE;
		offset ++;
	}
}



/* IDE hard disk driver using PIO (Programmed I/O) */

void
waitdisk()
{
	// wait for disk ready
	while ((inb(0x1F7) & 0xC0) != 0x40)
		/* do nothing */;
}

void
readsect(void *dst, uint32_t offset)
{
	// wait for disk to be ready
	waitdisk();
	
	outb(0x1F2, 1);		// count = 1
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8);
	outb(0x1F5, offset >> 16);
	outb(0x1F6, ((offset >> 24) & 0x0F) | 0xE0);
	outb(0x1F7, 0x20);	// cmd 0x20 - read sectors

	// wait for disk to be ready
	waitdisk();

	// read a sector
	insl(0x1F0, dst, SECTSIZE/4);	
}
