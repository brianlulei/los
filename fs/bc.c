#include <fs/fs.h>

static void
bc_pgfault(struct UTrapframe *utf)
{

}

void
bc_init(void)
{
	Super super;
	set_pgfault_handler(bc_pgfault);
}
