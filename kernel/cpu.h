#ifndef KERNEL_CPU_H_
#define KERNEL_CPU_H_

#include <include/types.h>
#include <include/env.h>
#include <include/mmu.h>

// Maximum number of CPUs
#define NCPU	8

// Values of status in struct Cpu
enum {
	CPU_UNUSED = 0,
	CPU_STARTED,
	CPU_HALTED,
};

// Per-CPU state
typedef struct {
	uint8_t				cpu_id;			// Local APIC ID: index into cpus[] below
	volatile unsigned	cpu_status;		// The states of the CPU
	Env					*cpu_env;		// The currently-running environment
	Taskstate			cpu_ts;			// Used by x86 to find stack for interrupt
} CpuInfo;

// Initialized in mpconfig.c
extern CpuInfo		cpus[NCPU];
extern int			ncpu;				// Total number of CPUs in the system
extern CpuInfo		*bootcpu;			// The boot-strap processor (BSP)
extern physaddr_t	lapicaddr;			// Physical MIMO address of the local APIC

int cpunum(void);
#define thiscpu (&cpus[cpunum()])

void mp_init(void);
void lapic_init(void);

#endif
