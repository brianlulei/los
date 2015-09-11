#include <fs/fs.h>


/* Fault any disk block that is read in to memory by 
 * loading it from disk.
 */
static void
bc_pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t blockno = ((uint32_t) addr - DISKMAP) / BLKSIZE;
	int r;

	// Check that the fault was within the block cache region
	if (addr < (void *)DISKMAP || addr >= (void *)(DISKMAP + DISKSIZE))
		panic("page fault in FS: eip %08x, va %08x, err %04x",
			   utf->utf_eip, addr, utf->utf_err);

	// Sanity check the block number.
	if (super && blockno >= super->s_nblocks)
		panic("reading non-existent block %08x\n", blockno);

	// Allocate a page in the disk map region, read the contents
	// of the block from the disk into that page.
	// Hint: first round addr to page boundary. fs/ide.c has code
	// to read the disk.
	addr = ROUNDDOWN(addr, PGSIZE);

	if ((r = sys_page_alloc(0, addr, PTE_U | PTE_W | PTE_P)) < 0)
		panic("bc_pgfault: no phys mem %e", r);

	if ((r = ide_read(blockno * BLKSECTS, addr, BLKSECTS)) < 0)
		panic("bc_pgfault: ide read error %e", r);

	// Clear the dirty bit for the disk block page since we
	// just read the block from disk
	if ((r = sys_page_map(0, addr, 0, addr, uvpt[PGNUM(addr)] & PTE_SYSCALL)) < 0)
		panic("bc_pgfault: sys_page_map: %e", r);

	// Check that the block we read was allocated. Why do 
	// we do this after reading the block in?
	if (bitmap && block_is_free(blockno))
		panic("reading free block %08x\n", blockno);
}

void
bc_init(void)
{
	Super super;
	set_pgfault_handler(bc_pgfault);
}
