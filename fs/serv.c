#include <fs/fs.h>

void
umain(int argc, char ** argv)
{
	static_assert(sizeof(File) == 256);
	binaryname = "fs";
	cprintf("FS is running\n");
}
