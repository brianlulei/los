#ifndef _KERNEL_ENV_H
#define _KERNEL_ENV_H

#include <include/env.h>

extern Env * envs;
extern Env * curenv;

void env_init(void);
void env_init_percpu(void);

#endif
