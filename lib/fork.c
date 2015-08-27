// Implement fork from user space

#include <include/lib.h>

/* PTE_COW marks copy-on-write page table entries. */
/* It is one of the bits explicitly alocated to user processes (PTE_AVAIL) */
#define PTE_COW		0x800

/* Custom page fault handler - if faulting page is copy-on-write,
 * map in our own private writable copy.
 */
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;
	pte_t pte = uvpt[PGNUM(addr)];

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page. If not, panic.
	// Hint:
	//	Use the read-only page table mappings at uvpt (see <inc/memlayout.h>).

	if (!((err & FEC_WR) && (pte & PTE_COW)))
		panic("pgfault : pagefault %08x not (FEC_WR and PTE_COW).\n", err);

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//	You should make three system calls.
	r = sys_page_alloc(0, PFTEMP, PTE_U | PTE_P | PTE_W);

	if (r < 0)
		panic("pgfault: sys_page_alloc error -- %e.\n", r);

	void *rounded = ROUNDDOWN(addr, PGSIZE);
	memmove(PFTEMP, rounded, PGSIZE);

	r = sys_page_map(0, PFTEMP, 0, rounded, PTE_P|PTE_U|PTE_W);

	if (r < 0)
		panic("pgfault: sys_page_map error -- %e", r);

	r = sys_page_unmap(0, PFTEMP);

	if (r < 0)
		panic("pgfault: sys_page_unmap error -- %e", r);

	return;
}

/* Map our virtual page pn (address pn * PGSIZE) into the target envid
 * as the same virtual address. If the page is writable or copy-on-write,
 * the new mapping must be created copy-on-write, and then our mapping must
 * be marked copy-on-write as well. (Why do need to mark ours copy-on-write
 * again if it was already copy-on-write at he beginning of this function?)
 *
 * Returns: 0 on success, < 0 on error.
 * It is also OK to panic on error.
 */
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
    pte_t pte = uvpt[pn];
    void *va = (void *)(pn << PGSHIFT);

    // If the page is writable or copy-on-write,
    // the mapping must be copy-on-write ,
    // otherwise the new environment could change this page.
    if ((pte & PTE_W) || (pte & PTE_COW)) {
        if (sys_page_map(0, va, envid, va, PTE_COW | PTE_U | PTE_P))
            panic("duppage: map cow error");
        
        // Change permission of the page in this environment to copy-on-write.
        // Otherwise the new environment would see the change in this environment.
        if (sys_page_map(0, va, 0, va, PTE_COW | PTE_U | PTE_P))
            panic("duppage: change perm error");
    } else if (sys_page_map(0, va, envid, va, PTE_U | PTE_P))
        panic("duppage: map ro error");

	return 0;
}

/* User-level fork with copy-on-write
 * Set up our page fault handler appropriately.
 * Create a child.
 * Copy our address space and page fault handler setup to the child.
 * Then mark the child as runnable and return.
 *
 * Returns: Child's envid to the parent, 0 to the child, < 0 on error.
 * It is also OK to panic on error.
 *
 * Hint:
 *	User uvpd, uvpt, and duppage.
 *	Remember to fix "thisenv" in the child process.
 *	Neither user exception stack should ever be marked copy-on-write.
 *	so much allocate a new page for the child's user exception stack.
 */
envid_t
fork(void)
{
	envid_t	envid;
	uintptr_t va;
	int r;

	// set page fault handler
	set_pgfault_handler(pgfault);

	// allocate
	envid = sys_exofork();

	if (envid < 0)
		panic("fork: sys_exofork error -- %e.\n", envid);

	if (envid == 0) {
		// Child process
		// Fix 'thisenv'
		thisenv = &envs[ENVX(sys_getenvid())];
		cprintf("envid = %d\n", thisenv->env_id);
		return 0;
	}

	// We're the parent.
	for (va = UTEXT; va < USTACKTOP; va += PGSIZE){
		if ((uvpd[PDX(va)] & PTE_P) && 
			(uvpt[PGNUM(va)] & PTE_P) && 
			(uvpt[PGNUM(va)] & PTE_U))
		{
			duppage(envid, PGNUM(va));
		}
		// For pages that are not PTE_W or PTE_COW, ignore it.
	}

	// Create new exception stack.
	r = sys_page_alloc(envid, (void*) (UXSTACKTOP-PGSIZE), PTE_U | PTE_P | PTE_W);

	if (r < 0)
		panic("[%08x] fork: sys_page_alloc error -- %e.\n", thisenv->env_id, r);

	// Set child's page fault handler
	r = sys_env_set_pgfault_upcall(envid, thisenv->env_pgfault_upcall);
	if (r < 0)
		panic("[%08x] fork: sys_env_set_pgfault_upcall error -- %e.\n", thisenv->env_id, r);

	// Child is ready to run, make it RUNNABLE
	r = sys_env_set_status(envid, ENV_RUNNABLE);

	if (r < 0)
		panic("[%08x] fork: sys_env_set_status error -- %e", thisenv->env_id, r);

	return envid;	
}
