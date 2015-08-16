#ifndef INCLUDE_LIB_H_
#define INCLUDE_LIB_H_

#include <include/env.h>
#include <include/memlayout.h>
#include <include/stdio.h>
#include <include/types.h>
#include <include/string.h>
#include <include/assert.h>
#include <include/syscall.h>

// main user program
void	umain(int argc, char **argv);

// libmain.c or entry.S
extern const char *binaryname;
extern const volatile Env *thisenv;
extern const volatile Env envs[NENV];
extern const volatile PageInfo pages[];

// exit.c
void	exit(void);

//syscall.c	
void			sys_cputs(const char *string, size_t len);
int				sys_cgetc(void);
envid_t			sys_getenvid(void);
int				sys_env_destroy(envid_t);

void			sys_yield(void);
static envid_t	sys_exofork(void);
int				sys_env_set_status(envid_t env, int status);
int				sys_page_alloc(envid_t env, void *va, int perm);
int				sys_page_map(envid_t src_env, void *src_pg,
							 envid_t dst_env, void *dst_pg, int perm);
int				sys_page_unmap(envid_t env, void *pg);

// This must be inlined.
static __inline envid_t __attribute__((always_inline))
sys_exofork(void)
{
	envid_t ret;
	__asm __volatile("int %2"
		: "=a" (ret)
		: "a" (SYS_exofork),
		  "i" (T_SYSCALL)
	);
	return ret;
}

#endif
