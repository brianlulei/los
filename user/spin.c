// Test preemption by forking off a child process that just spins forever.
// Let it run for a couple time slices, then kill it.

#include <include/lib.h>

void
umain(int argc, char **argv)
{
    envid_t env;

    cprintf("edi = %d, I am the parent 1 .  Forking the child...\n", thisenv->env_id);
    if ((env = fork()) == 0) {
        cprintf("I am the child.  Spinning...\n");
    }   

    cprintf("eid = %d, I am the parent 2 .  Running the child...\n", thisenv->env_id);
	sys_yield();
    cprintf("eid = %d, I am the parent 3 .  Killing the child...\n", thisenv->env_id);
}
