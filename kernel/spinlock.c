#include <kernel/spinlock.h>
#include <kernel/cpu.h>
#include <kernel/kdebug.h>

#include <include/assert.h>
#include <include/x86.h>
#include <include/string.h>

/* The big kernel lock */
spinlock kernel_lock = {

#ifdef	DEBUG_SPINLOCK
	.name = "kernel_lock"
#endif

};

#ifdef DEBUG_SPINLOCK

/* Record the current call stack in pcs[] by following the %ebp chain */
static void
get_caller_pcs(uint32_t pcs[])
{
	uint32_t *ebp;
	int i;

	ebp = (uint32_t *) read_ebp();
	for (i = 0; i < 10; i++) {
		if (ebp == 0 || ebp < (uint32_t *) ULIM)
			break;
		pcs[i] = ebp[1];			// saved %eip
		ebp = (uint32_t *) ebp[0];	// saved %ebp
	}

	// Nullify the other pointers if less than 10.
	for (; i < 10; i++)
		pcs[i] = 0;
}

/* Check whether this CPU is holding the lock */
static int
holding(spinlock *lock)
{
	return lock->locked && lock->cpu == thiscpu;
}
#endif

void
__spin_initlock(spinlock *lk, char *name)
{
	lk->locked = 0;

#ifdef DEBUG_SPINLOCK
	lk->name = name;
	lk->cpu = 0;
#endif
}

/* Acquire the lock.
 * Loops (spins) until the lock is acquired.
 * Holding a lock for a long time may cause
 * other CPUs to waste time spinning to acquire it.
 */
void
spin_lock(spinlock *lk)
{
#ifdef	DEBUG_SPINLOCK
	if (holding(lk))
		panic("CPU %d cannot acquire %s: already holding", cpunum(), lk->name);
#endif

	// The xchg is atomic.
	while (xchg(&lk->locked, 1) != 0)
		asm volatile ("pause");

#ifdef DEBUG_SPINLOCK
	lk->cpu = thiscpu;
	
#endif
}

/* Release the lock */
void
spin_unlock(spinlock *lk)
{
#ifdef	DEBUG_SPINLOCK
	if (!holding(lk)) {
		int i;
		uint32_t pcs[10];

		memmove(pcs, lk->pcs, sizeof pcs);
		cprintf("CPU %d cannot release %s: held by CPU %d\n Acquired at:",
				cpunum(), lk->name, lk->cpu->cpu_id);

		for (i = 0; i < 10 && pcs[i]; i++) {
			Eipdebuginfo info;
			if (debuginfo_eip(pcs[i], &info) >= 0)
                cprintf("  %08x %s:%d: %.*s+%x\n", pcs[i],
						info.eip_file, info.eip_line,
						info.eip_fn_namelen, info.eip_fn_name,
						pcs[i] - info.eip_fn_saddr);
            else
                cprintf("  %08x\n", pcs[i]);
		}
		panic("spin_unlock");
	}
	lk->pcs[0] = 0;
	lk->cpu = 0;
#endif

	xchg(&lk->locked, 0);	
}

