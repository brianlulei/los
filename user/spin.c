// Test preemption by forking off a child process that just spins forever.
// Let it run for a couple time slices, then kill it.

#include <include/lib.h>

void
umain(int argc, char **argv)
{
    envid_t env;

    cprintf("I am the parent.  Forking the child...\n");
    if ((env = fork()) == 0) {
        cprintf("I am the child.  Spinning...\n");
		cprintf("envid 1 = %d\n", thisenv->env_id);
    }   

	cprintf("envid 2 = %d\n", thisenv->env_id);
    cprintf("I am the parent.  Running the child...\n");
	sys_yield();
	cprintf("envid 3 = %d\n", thisenv->env_id);
    cprintf("I am the parent.  Killing the child...\n");
}
