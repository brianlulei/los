#ifndef INCLUDE_LIB_H_
#define INCLUDE_LIB_H_

#include <include/env.h>
#include <include/memlayout.h>
#include <include/stdio.h>
#include <include/types.h>
#include <include/string.h>

// main user program
void	umain(int argc, char **argv);

// libmain.c or entry.S
extern const char *binaryname;
extern const volatile Env *thisenv;
extern const volatile Env envs[NENV];
extern const volatile PageInfo pages[];

// exit.c
void	exit(void);



#endif
