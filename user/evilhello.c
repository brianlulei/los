#include <include/lib.h>

void
umain(int argc, char **argv)
{
	// try to print the kernel entry point as a string! mua ha ha!
	sys_cputs((char *)0xf010000c, 100);
}
