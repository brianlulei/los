#ifndef KERNEL_SPINLOCK_H_
#define KERNEL_SPINLOCK_H_

#include <include/types.h>
#include <kernel/cpu.h>

// Comment this to disable spinlock debugging
#define DEBUG_SPINLOCK

// Mutual exclusion lock.
typedef struct {
	unsigned	locked;		// Is the lock held?

#ifdef DEBUG_SPINLOCK
	// For debugging:
	char		*name;		// Name of lock.
	CpuInfo		*cpu;		// The CPU holding the lock.
	uintptr_t	pcs[10];	// The call stack (an array of program counters)
							// that locked the lock.
#endif
} spinlock;

void __spin_initlock(spinlock *lk, char *name);
void spin_lock(spinlock *lk);
void spin_unlock(spinlock *lk);

#define spin_initlock(lock)		__spin_initlock(lock, #lock)

extern spinlock kernel_lock;

static inline void
lock_kernel(void)
{
	spin_lock(&kernel_lock);
}

static inline void
unlock_kernel(void)
{
	spin_unlock(&kernel_lock);

	// Normally we wouldn't need to do this, but QEMU only runs
	// one CPU at a time and has a long time-slice. Without the
	// pause, this CPU is likely to reacquire the lock before
	// another CPU has been given a change to acquire it.
	asm volatile("pause");
}

#endif
