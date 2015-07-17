#include <include/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("hello, world\n");
	cprintf("I am environment %08x\n", thisenv->env_id);
}
