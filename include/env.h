#ifndef _ENV_H_
#define _ENV_H_

#include <include/types.h>
#include <include/trap.h>
#include <include/memlayout.h>

typedef int32_t envid_t;

/* 
 * An environment ID 'envid_t' has three parts:
 *
 * +1+-----------------21--------------------+---------10---------+
 * |0|         Uniqueifier                   |   Environment      |
 * | |                                       |      Index         |
 * +-----------------------------------------+--------------------+
 *                                            \ --- ENVX(eid) ---/
 *
 * The environment index ENVX(eid) queals the environment's offset in the 
 * 'env[]' array. The uniqueifier distinguishes environments that were
 * created at different times, but share the same environment index.
 *
 * All real environments are greater than 0 ( so the sign bit is zero).
 * envid_ts less than 0 signify errors. The envid_t == 0 is special, and
 * stands for the current environment.
 */

#define LOG2NENV		10
#define NENV			(1 << LOG2NENV)
#define ENVX(envid)		((envid) & (NENV - 1))

// Values of env_status in struct Env
enum {
	ENV_FREE = 0,
	ENV_DYING,
	ENV_RUNNABLE,
	ENV_RUNNING,
	ENV_NOT_RUNNNABLE
};

// Special environment types
enum EnvType {
	ENV_TYPE_USER = 0,
};

struct Env{
	struct Trapframe	env_tf;			// saved registers
	struct Env *		env_link;		// next free Env
	envid_t				env_id;			// Unique environment identifier
	envid_t				env_parent_id;	// env_id of this env's parent
	enum EnvType		env_type;		// indicates special system environments
	unsigned			env_status;		// status of the environment
	uint32_t			env_runs;		// number of times environment has run

	// Address space
	pde_t *				env_pgdir;		// Kernel virtual address of page directory	
};

typedef struct Env Env;
#endif
