#include <include/string.h>
#include <fs/fs.h>


// --------------------------------------------------------------
// File system structures
// --------------------------------------------------------------

/* Initialize the file system */
void
fs_init(void)
{
	static_assert(sizeof(File) == 256);

	// Find a second disk. Use the second IDE (number 1) if available
	if (ide_probe_disk1())
		ide_set_disk(1);
	else
		ide_set_disk(0);

	bc_init();
}
