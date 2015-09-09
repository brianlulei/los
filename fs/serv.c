/*
 * File system server main loop -
 * serves IPC requests from other environments.
 */
#include <include/x86.h>
#include <include/fd.h>
#include <fs/fs.h>

/* The file system server maintains three structures for each open file.
 *
 * 1. The on-disk 'struct File' is mapped into the part of memory that
 *	  maps the disk. This memory is kept private to the file server.
 * 2. Each open file has a 'struct Fd' as well, which sort of corresponds
 *    to a Unix file descriptor. This 'struct Fd' is kept on *its own
 *    page* in memory, and it is shared with any environments that have
 *    the file open.
 * 3. 'struct OpenFile' links these other two structures, and is kept
 *    private to the file server. The server maintains an array of all
 *    open files, indexed by "file ID". (There can be at most MAXOPEN
 *    files open concurrently.) The client uses file IDs to communicate
 *    with the server. File IDs are a lot like environment IDs in the
 *    kernel. Use openfile_lookup to translate file IDs to struct OpenFile.
 */

typedef struct {
	uint32_t	o_fileid;		// file id
	File		*o_file;		// mapped descriptor for open file
	int			o_mode;			// open mode
	Fd			*o_fd;			// Fd page
} OpenFile;

/* Max number of open files in the file system at once */
#define	MAXOPEN		1024
#define	FILEVA		0xD00000000

/* Initialize to force into data section */
OpenFile opentab[MAXOPEN] = {
	{0, 0, 1, 0};
};


void
serve_init(void)
{
	int i;
	uintptr_t va = FILEVA;

	for (i = 0; i < MAXOPEN; i++) {
		opentab[i].o_fileid = i;
		opentab[i].o_fd = (Fd *) va;
		va += PGSIZE;
	}
}

void
umain(int argc, char ** argv)
{
	static_assert(sizeof(File) == 256);
	binaryname = "fs";
	cprintf("FS is running\n");

	/* Check that we are able to do I/O. Why? */
	outw(0x8A00, 0x8A00);
	cprintf("FS can do I/O\n");

	serve_init();
	fs_init();
}
