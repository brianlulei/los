#include <include/types.h>
#include <include/stdio.h>
#include <include/assert.h>
#include <include/syscall.h>
#include <include/error.h>
#include <include/string.h>
#include <include/mmu.h>

#include <kernel/console.h>
#include <kernel/env.h>
#include <kernel/pmap.h>
#include <kernel/cpu.h>
#include <kernel/sched.h>

static void
sys_cputs(const char *s, size_t len)
{
	// Check that the user has permission to read memory [s, s+len).
	// Destroy the environment if not.
	
	user_mem_assert(curenv, s, len, 0);

	// Print the string supplied by the user.
	cprintf("%.*s", len, s);
}

/* Read a character from the system console without blocking.
 * Returns the character, or 0 if there is no input waiting.
 */
static int
sys_cgetc(void)
{
	return cons_getc();
}

/* Returns the current environment's envid. */
static envid_t
sys_getenvid(void)
{
	return curenv->env_id;
}

/* Destroy a given environment
 * Returns 0 on success, < 0 on error.
 * Errors are:
 *  -E_BAD_ENV if environment envid doesn't currently exist,
 *			   or the caller doesn't have permission to change envid.
 */
static int
sys_env_destroy(envid_t envid)
{
	int r;
	Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0)
		return r;

	if (e == curenv)
		cprintf("[%08x] exiting gracefully\n", curenv->env_id);
	else
		cprintf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);

	env_destroy(e);
	return 0;
}

/* Deschedule current environment and pick a different one to run */
static void
sys_yield(void)
{
	sched_yield();
}

/* Allocate a new envirionment.
 * Returns envid of new environment, or < 0 on error. Errors are:
 *	-E_NO_FREE_ENV	if no free environment is available.
 *	-E_NO_MEM on memory exhaustion.
 */
static envid_t
sys_exofork(void)
{
	// Create the new environment with env_alloc(), from kernel/env.c.
	// It should be left as env_alloc created it, except that
	// status is set to ENV_NOT_RUNNABLE, and the register set is copied
	// from the current environment -- but the tweaked so sys_exofork
	// will appear to return 0.
	Env *child_env;
	int ret = env_alloc(&child_env, curenv->env_id);

	if (ret < 0)
		return -E_NO_FREE_ENV;

	// Set trapframe. 
	// Child's eax should be zero because child env's fork returns zero.
	memmove(&child_env->env_tf, &curenv->env_tf, sizeof (struct Trapframe));
	child_env->env_tf.tf_regs.reg_eax = 0;

	// Status is set to ENV_NOT_RUNNABLE
	child_env->env_status = ENV_NOT_RUNNABLE;
	return child_env->env_id;
}

/* Set envid's env_status to status, which must be ENV_RUNNABLE
 * or ENV_NOT_RUNNABLE.
 *
 * Returns 0 on success, < 0 or error. Errors are:
 *	-E_BAD_ENV	if environment envid doesn't currently exist,
 *				or the caller doesn't have permission to change envid.
 *	-E_INVAL	if status is not a valid status for an environment. 
 */
static int
sys_env_set_status(envid_t envid, int status)
{
	// Hint: Use the 'envid2env' function from kernel/env.c to translate an
	// envid to a struct Env.
	// Set envid2env's third argument to 1, which will check whether the
	// current environment has permission to set envid's status.
	Env *env;

	int ret = envid2env(envid, &env, 1);
	if (ret < 0)
		return -E_BAD_ENV;

	if (status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE)
		return -E_INVAL;

	env->env_status = status; 
	return 0;
}

/* Set the page fault upcall for 'envid' by modifying the corresponding struct
 * Env's 'env_pgfault_upcall' field. When 'envid' causes a page fault, the 
 * kernel will push a fault record onto the exception stack, then branch to 'func'.
 *
 * Returns 0 on success, < 0 on error. Errors are:
 *	-E_BAD_ENV	if environment envid doesn't currently exist,
 *				or the caller doesn't have permission to change envid.
 */
static int
sys_env_set_pgfault_upcall(envid_t envid, void *func)
{
	Env *env;
	int ret = envid2env(envid, &env, 1);

	if (ret < 0)
		return -E_BAD_ENV;

	env->env_pgfault_upcall = func;
	return 0;
}

/* Allocate a page of memory and map it at 'va' with permission
 * 'perm' in the address space of 'envid'.
 * The page's contents are set to 0.
 * If a page is already mapped at 'va', that page is unmapped as a side effect.
 *
 * perm -- PTE_U | PTE_P must be set, PTE_AVAIL | PTE_W may or may not be set,
 *		   but no other bits may be set. See PTE_SYSCALL in include/mmu.h.
 *
 * Returns 0 on success, < 0 on error. Errors are:
 *	-E_BAD_ENV	if environment envid doesn't currently exist,
 *				or the caller doesn't have permission to change envid.
 *	-E_INVAL	if va >= UTOP, or va is not page-aligned.
 *  -E_INVAL	if perm is inappropriate (see above.)
 *  -E_NO_MEM	if there's no memory to allocate the new page, 
 *				or to allocate any necessary page tables.
 */
static int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	// Hint: This function is a wrapper around page_alloc() and
	// page_insert() from kernel/pmap.c.
	// Most of the new code should be to check the parameters for correctness.
	// If page_insert() fails, remember to free the page allocated.
	Env *env;
	int ret;
	PageInfo *pp;

	// Check envid
	ret = envid2env(envid, &env, 1);
	if (ret < 0)
		return -E_BAD_ENV;

	// Check va
	if ((uintptr_t)va >= UTOP || PGOFF(va))
		return -E_INVAL;

	// Check perm
	if (((perm | PTE_SYSCALL) != PTE_SYSCALL) ||		// no other bits may be set
		((perm | PTE_U | PTE_P) != perm))				// PTE_U | PTE_P must be set
		return -E_INVAL;

	// Allocate page and set to ZERO
	pp = page_alloc(ALLOC_ZERO);
	if (!pp)
		return -E_NO_MEM;

	// Insert into page table
	ret = page_insert(env->env_pgdir, pp, va, perm);
	if (ret < 0) {
		page_free(pp);
		return -E_NO_MEM;
	}
	return 0;
}

/* Map the page of memory at 'srcva' in srcenvid's address space
 * at 'dstva' in dstenvid's address space with permission 'perm'.
 * Perm has the same restriction as in sys_page_alloc, except
 * that it also must not grant write access to a read-only page.
 *
 * Returns 0 on success, < 0 on error. Errors are:
 *	-E_BAD_ENV	if srcenvid and/or dstenvid doesn't currently exist,
 *				or the caller doesn't have permission to change one of them.
 *	-E_INVAL	if srcva >= UTOP or srcva is not page-aligned,
 *				or dstva >= UTOP or dstva is not page-aligned.
 *	-E_INVAL	if srcva is not mapped in srcenvid's address space.
 *	-E_INVAL	if perm is inappropriate (see sys_page_alloc).
 *	-E_INVAL	if (perm & PTE_W), but srcva is read-only in srcenvid's
 *				address space.
 *	-E_NO_MEM	if there's no memory to allocate any necessary page tables.
 */
static int
sys_page_map(envid_t srcenvid, void *srcva,
			 envid_t dstenvid, void *dstva, int perm)
{
	// Hint: This function is a wrapper around page_lookup() and
	//		 page_insert() from kernel/pmap.c.
	//		 Check the paramters for correnctness.
	//		 User the third argument to page_lockup()
	//		 to check the current permissions on the page.	
	Env *src_env, *dst_env;
	int ret;
	PageInfo *pp;
	pte_t *src_pte;

	// Check srcenvid and dstenvid
	ret = envid2env(srcenvid, &src_env, 1);
	if (ret < 0)
		return -E_BAD_ENV;

	ret = envid2env(dstenvid, &dst_env, 1);
	if (ret < 0)
		return -E_BAD_ENV;

	// Check srcva and dstva
	if ((uintptr_t)srcva >= UTOP || (uintptr_t)dstva >= UTOP
		|| PGOFF(srcva) || PGOFF(dstva))
		return -E_INVAL;

	// Check srcva is mapped in srcenvid's address space
	pp = page_lookup(src_env->env_pgdir, srcva, &src_pte);
	if (!pp)
		return -E_INVAL;

	// Check perm is appropriate
	if (((perm | PTE_SYSCALL) != PTE_SYSCALL) ||        // no other bits may be set
		((perm | PTE_U | PTE_P) != perm))               // PTE_U | PTE_P must be set
		return -E_INVAL;

	// Check read-only for srcenvid
	if ((perm & PTE_W) && !(*src_pte & PTE_W))
		return -E_INVAL;

	// Insert into page table
	ret = page_insert(dst_env->env_pgdir, pp, dstva, perm);
	if (ret < 0)
		return -E_NO_MEM;

	return 0;
}

/* Unmap the page of memory at 'va' in the address space of 'envid'.
 * If no page is mapped, the functino silently succeeds.
 *
 * Returns 0 on success, < 0 on error. Errors are:
 *	-E_BAD_ENV	if environment envid doesn't currently exist,
 *				or the caller doesn't have permission to change envid.
 *	-E_INVAL	if va >= UTOP, or va is not page-aligned.
 */
static int
sys_page_unmap(envid_t envid, void *va)
{
	// Hint: This function is a wrapper around page_remove().
	int ret;
	Env *env;

	// Check envid
	ret = envid2env(envid, &env, 1);
	if (ret < 0)
		return -E_BAD_ENV;

	// Check va
	if ((uintptr_t)va >= UTOP || PGOFF(va))
		return -E_INVAL;

	// Do page unmap
	page_remove(env->env_pgdir, va);
	return 0;
}

/* Try to send 'value' to the target env 'envid'
 * If srcva < UTOP, then also send page currently mapped at 'srcva',
 * so that receiver gets a duplicate mapping of the same page.
 *
 * The send fails with a return value of -E_IPC_NOT_RECV if the target
 * is not blocked, waiting for an IPC.
 *
 * The send also can fail for the other reasons listed below.
 *
 * Otherwise, the send succeeds, and the target's ipc fields are
 * updated as follows:
 *		env_ipc_receiving is set to 0 to block future sends;
 *		env_ipc_from is set to the sending envid;
 *		env_ipc_value is set to the 'value' parameter;
 *		env_ipc_perm is set to 'perm' if a page was transferred, 0 otherwise.
 * The target environment is marked runnable again, returning 0
 * from the paused sys_ipc_recv system call. (Hint: does the sys_ipc_recv
 * function ever atually return?)
 *
 * If the sender wants to send a page but the receiver isn't asking for one,
 * then no page mapping is transferred, but no error occurs.
 * The ipc only happens when no errors occur.
 *
 * Returns 0 on success, < 0 on error. Errors are:
 *	-E_BAD_ENV	if environment envid doesn't curently exist.
 *				(No need to check permissions.)
 *	-E_IPC_NOT_RECV	if envid is not currently blocked in sys_ipc_recv,
 *					or another environment managed to send first.
 *	-E_INVAL	if srcva < UTOP but srcva is not page-aligned.
 *	-E_INVAL	if srcva < UTOP and perm is inappropiate (see sys_page_alloc).
 *	-E_INVAL	if srcva < UTOP but srcva is not mapped in the caller's
 *				address space.
 *	-E_INVAL	if (perm & PTE_W), but srcva is read-only in the current
 *				environment's address space.
 *	-E_NO_MEM	if there's not enough memory to map srcva in envid's
 *				address space.
 */
static int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
	int ret;
	Env *dstenv;
	PageInfo *pp;
	pte_t *pte;

	// check parameters
	// check envid and env status
	ret = envid2env(envid, &dstenv, 0);

	if (ret < 0)
		return -E_BAD_ENV;

	if (!dstenv->env_ipc_recving)
		return -E_IPC_NOT_RECV;

	// Check srcva and perm
	if ((uintptr_t)srcva < UTOP) {
		if (PGOFF(srcva) != 0)
			return -E_INVAL;

		if ((perm & (PTE_U | PTE_P)) != (PTE_U | PTE_P))
			return -E_INVAL;

		if (((perm | PTE_SYSCALL) != PTE_SYSCALL))
			return -E_INVAL;

		// Check physical page exist
		pp = page_lookup(curenv->env_pgdir, srcva, &pte);

		if ((uintptr_t)srcva < UTOP && !pp)
			return -E_INVAL;

		// Check perm write conflict
		if ((perm & PTE_W) && !(*pte & PTE_W))
			return -E_INVAL;

		// Send mapping
		if ((uintptr_t) dstenv->env_ipc_dstva < UTOP) {
			// Do page map
			ret = page_insert(dstenv->env_pgdir, pp, dstenv->env_ipc_dstva, perm);

			if (ret < 0)
				return -E_NO_MEM;
			// Make page perm
			dstenv->env_ipc_perm = perm;
		}
    }
	// If srcva >= UTOP, no mapping transfered and no errors.
	dstenv->env_ipc_recving = false;
	dstenv->env_ipc_value = value;
	dstenv->env_ipc_from = curenv->env_id;
	dstenv->env_status = ENV_RUNNABLE;

	return 0;
}

/* Block until a value is ready. Record that you want to receive
 * using the env_ipc_recving and env_ipc_dstva fields of struct Env,
 * mark yourself not runnable, and then give up the CPU.
 *
 * If 'dstva' is < UTOP, then you are willing to receive a page of data.
 * 'dstva' is the virtual address at which the sent page should be mapped.
 *
 * This function only returns on error, but the system call will eventyally
 * return 0 on success.
 * Return < 0 on error. Errors are:
 *	-E_INVAL	if dstva < UTOP but dstva is not page-aligned.
 */
static int
sys_ipc_recv(void *dstva)
{
	// check dstva
	if ((uintptr_t)dstva < UTOP) {
		if (PGOFF(dstva) != 0)
			return -E_INVAL;

		curenv->env_ipc_dstva = dstva;
	} else {
		curenv->env_ipc_dstva = (void *)UTOP;
	}

	// Record this env want to receive
	curenv->env_ipc_recving = true;
	curenv->env_ipc_dstva = dstva;

	// Block this env, and giveup CPU
	curenv->env_status = ENV_NOT_RUNNABLE;

	return 0;
}

/* Dispatches to the correct kernel function, passing the arguments. */
int32_t
syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
    // Call the function corresponding to the 'syscallno' parameter.
    // Return any appropriate return value.

    switch (syscallno) {
		case SYS_cputs:
			sys_cputs((const char *) a1, a2);
			return 0;
		case SYS_cgetc:
			return sys_cgetc();
		case SYS_getenvid:
			return sys_getenvid();
		case SYS_env_destroy:
			return sys_env_destroy(a1);			
		case SYS_exofork:
			return (int32_t) sys_exofork();
		case SYS_yield:
			sys_yield();
		case SYS_env_set_status:
			return (int32_t) sys_env_set_status((envid_t) a1, a2);
		case SYS_page_alloc: 
			return (uint32_t) sys_page_alloc((envid_t)a1, (void *)a2, (int)a3);
		case SYS_page_map:
			return (uint32_t) sys_page_map((envid_t)a1, (void *)a2,
										   (envid_t)a3, (void *)a4, (int)a5);
		case SYS_page_unmap:
			return (uint32_t) sys_page_unmap((envid_t)a1, (void *)a2);
		case SYS_env_set_pgfault_upcall:
			return (uint32_t) sys_env_set_pgfault_upcall((envid_t)a1, (void*)a2);
		case SYS_ipc_try_send:
			return (uint32_t) sys_ipc_try_send((envid_t) a1, a2, (void *) a3, a4);
		case SYS_ipc_recv:
			return (uint32_t) sys_ipc_recv((void *) a1);
		default:
			return -E_NO_SYS;
    }
}
