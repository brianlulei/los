#include <include/lib.h>

void
exit(void)
{
	sys_env_destroy(0);
}
