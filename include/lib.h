#ifndef INCLUDE_LIB_H_
#define INCLUDE_LIB_H_

#include <include/env.h>
#include <include/memlayout.h>
#include <include/stdio.h>
#include <include/types.h>
#include <include/string.h>
#include <include/assert.h>

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
void	sys_cputs(const char *string, size_t len);
int		sys_cgetc(void);
envid_t	sys_getenvid(void);
int		sys_env_destroy(envid_t);

#endif
