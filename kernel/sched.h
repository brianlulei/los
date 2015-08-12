#ifndef KERNEL_SCHED_H_
#define KERNEL_SCHED_H_

#ifndef LOS_KERNEL
#error "This is a LOS kernel header; user programs should not #include it"
#endif

// This function does not return.
void	sched_yield(void)	__attribute__((noreturn));

#endif
