/*
 * File system server main loop -
 * serves IPC requests from other environments.
 */
#include <include/x86.h>
#include <include/string.h>
#include <include/fd.h>
#include <fs/fs.h>

#define debug 0

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
#define	FILEVA		0xD0000000

/* Initialize to force into data section */
OpenFile opentab[MAXOPEN] = {
	{0, 0, 1, 0}
};

/* Virtual address at which to receive page mappings containing client requests. */
union Fsipc *fsreq = (union Fsipc *) 0x0ffff000;

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

/* Open req->req_path in mode req->req_omode, storing the Fd page and
 * permissions to return to the calling environment in *pg_store and
 * *perm_store respectively.
 */
int
serve_open(envid_t envid, struct Fsreq_open *req,
       void **pg_store, int *perm_store)
{
	return 0;
}

/* Set the size of req->req_fileid to req->req_size bytes, truncating
 * or extending the file as necessary.
 */
int
serve_set_size(envid_t envid, struct Fsreq_set_size *req)
{
	return 0;
}

/* Read at most ipc->read.req_n bytes from the current seek position
 * in ipc->read.req_fileid.  Return the bytes read from the file to
 * the caller in ipc->readRet, then update the seek position.  Returns
 * the number of bytes successfully read, or < 0 on error.
 */
int
serve_read(envid_t envid, union Fsipc *ipc)
{
	return 0;
}

/* Write req->req_n bytes from req->req_buf to req_fileid, starting at
 * the current seek position, and update the seek position
 * accordingly.  Extend the file if necessary.  Returns the number of
 * bytes written, or < 0 on error.
 */
int
serve_write(envid_t envid, struct Fsreq_write *req)
{
	return 0;
}

/* Stat ipc->stat.req_fileid.  Return the file's struct Stat to the
 * caller in ipc->statRet.
 */
int
serve_stat(envid_t envid, union Fsipc *ipc)
{
	return 0;
}

/* Flush all data and metadata of req->req_fileid to disk. */
int
serve_flush(envid_t envid, struct Fsreq_flush *req)
{
	return 0;
}

int
serve_sync(envid_t envid, union Fsipc *req)
{
	return 0;
}


/* Define the 'fshandler' type to be a pointer to function that returning an int */
typedef int (* fshandler)(envid_t envid, union Fsipc *req);

fshandler handlers[] = {
	// Open is handled specially because it passes pages
	// [FSREQ_OPEN] = (fshandler) serve_open
	[FSREQ_READ] =		serve_read,
	[FSREQ_STAT] =		serve_stat,
	[FSREQ_FLUSH] =		(fshandler) serve_flush,
	[FSREQ_WRITE] =		(fshandler) serve_write,
	[FSREQ_SET_SIZE] =	(fshandler) serve_set_size,
	[FSREQ_SYNC] =		serve_sync
};

#define NHANDLERS (sizeof(handlers) / sizeof(handlers[0]))

void
serve(void)
{
	uint32_t req, whom;
	int perm, r;
	void *pg;

	while (1) {
		req = ipc_recv((int32_t *) &whom, fsreq, &perm);
		if (debug)
			cprintf("fs req %d from %08x [page %08x: %s]\n",
					req, whom, uvpt[PGNUM(fsreq)], fsreq);

		// All requests must contain an argument page
		if (!(perm & PTE_P)) {
			cprintf("Invalid request from %08x: no argument page\n", whom);
			continue;
		}

		pg = NULL;
		if (req == FSREQ_OPEN) {
			r = serve_open(whom, (struct Fsreq_open *)fsreq, &pg, &perm);
		} else if (req < NHANDLERS && handlers[req]) {
			r = handlers[req](whom, fsreq);
		} else {
			cprintf("Invalid request code %d from %08x\n", req, whom);
			r = -E_INVAL;
		}
		ipc_send(whom, r, pg, perm);
		sys_page_unmap(0, fsreq);
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
	fs_test();
	serve();
}
