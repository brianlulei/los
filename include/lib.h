#ifndef INCLUDE_LIB_H_
#define INCLUDE_LIB_H_

#include <include/env.h>
#include <include/memlayout.h>
#include <include/stdio.h>
#include <include/types.h>
#include <include/string.h>
#include <include/assert.h>
#include <include/syscall.h>
#include <include/error.h>

// main user program
void	umain(int argc, char **argv);

// libmain.c or entry.S
extern const char *binaryname;
extern const volatile Env *thisenv;
extern const volatile Env envs[NENV];
extern const volatile PageInfo pages[];

// exit.c
void	exit(void);

// pgfault.c
void    set_pgfault_handler(void (*handler)(struct UTrapframe *utf));

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

int				sys_env_set_pgfault_upcall(envid_t env, void *upcall);

int				sys_ipc_try_send(envid_t to_env, uint32_t value, void *pg, int perm);
int				sys_ipc_recv(void *rcv_pg);

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

// ipc.c
void		ipc_send(envid_t to_env, uint32_t value, void *pg, int perm);
int32_t		ipc_recv(envid_t *from_env_store, void *pg, int *perm_store);
envid_t		ipc_find_env(enum EnvType type);

// fork.c
#define PTE_SHARE		0x400
envid_t fork(void);
envid_t sfork(void);


#endif
