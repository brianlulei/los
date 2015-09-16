#include <include/string.h>
#include <fs/fs.h>


// --------------------------------------------------------------
// Super block
// --------------------------------------------------------------

/* Validate the file system super-block. */
void
check_super(void)
{
	if (super->s_magic != FS_MAGIC)
		panic("bad file system magic number");

	if (super->s_nblocks > DISKSIZE / BLKSIZE)
		panic("file system is too large");

	cprintf("superblock is good\n");
}

// --------------------------------------------------------------
// Free block bitmap
// --------------------------------------------------------------

/* Check to see i the block bitmap indicates that block 'blockno' is free.
 * Return 1 if the block is free, 0 if not.
 */
bool
block_is_free(uint32_t blockno)
{
	if (super == 0 || blockno >= super->s_nblocks)
		return 0;
	if (bitmap[blockno / 32] & (1 << (blockno % 32)))
		return 1;
	return 0;
}

/* Mark a block free in the bitmap */
void
free_block(uint32_t blockno)
{
	// Blockno zero is the null pointer of block numbers.
	if (blockno == 0)
		panic("attempt to free zero block");
	bitmap[blockno/32] |= 1 << (blockno % 32);
}

/* Mark a block allocated in the bitmap */
static void
unfree_block(uint32_t blockno)
{
	// Blockno zero is the null pointer of block numbers.
	if (blockno == 0)
		panic("unfree_block: attempt to free zero block");
	bitmap[blockno/32] &= ~(1 << (blockno % 32));
}

/* Search the bitmap for a free block and allocate it. When you
 * allocate a block, immediately flush the changed bitmap block to disk.
 *
 * Return block number allocated on success,
 * -E_NO_DISK if we are out of blocks.
 *
 * Hint: use free block as an example for manipulating the bitmap.
 */
int alloc_block(void)
{
	// Th bitmap consists of one or more blocks. A single bitmap block
	// contains the in-use bits for BLKBITSIZE blocks. There are
	// super->s_nblocks blocks in the disk altogether.
	uint32_t blockno, blockbitno;

	for (blockno = 0; blockno < super->s_nblocks; blockno++) {
		if (block_is_free(blockno)) {
			unfree_block(blockno);
			blockbitno = blockno / BLKBITSIZE;
			flush_block(diskaddr(2 + blockbitno));
			return blockno;	
		}
	}
	return -E_NO_DISK;
}


/* Validate the file system bitmap. 
 *
 * Check that al reserved blocks -- 0, 1, and the bitmap blocks themselves --
 * are all marked as in use.
 */
void
check_bitmap(void)
{
	uint32_t i;

	// Make sure all bitmap blocks are marked in-use
	for (i = 0; i * BLKBITSIZE < super->s_nblocks; i++)
		assert(!block_is_free(2 + i));

	// Make sure the reserved and root blocks are marked in-use
	assert(!block_is_free(0));
	assert(!block_is_free(1));

	cprintf("bitmap is good\n");
}

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

	// Set "super" to point to the super block.
	super = diskaddr(1);
	check_super();

	// Set "bitmap" to the beginning of the first bitmap block.
	bitmap = diskaddr(2);
	check_bitmap();
}

/* Skip over slashes. */
static const char*
skip_slash(const char *p)
{
	while (*p == '/')
		p++;
	return p;
}

/* Find the disk block number slot for the 'filebno'th block in file 'f'
 * Set '*ppdiskbno' to point to that slot.
 * The slot will be one of the f->f_direct[] entries,
 * or an entry in the indirect block.
 * When 'alloc' is set, this function will allocate an indirect block
 * if necessary.
 *
 * Returns:
 *	0 on success (but note that *ppdiskbno might equal 0).
 *	-E_NOT_FOUND if the function needed to allocated an indirect block, but
 *		alloc was 0.
 *	-E_NO_DISK if there's no space on the disk for an indirect block.
 *	-E_INVAL if filebno is out of range (it's >= NDIRECT + NINDIRECT).
 *
 * Analogy: This is like pgdir_walk for files.
 * Hint: Don't forget to clear any block you allocated.
 */
static int
file_block_walk(File *f, uint32_t filebno, uint32_t **ppdiskbno, bool alloc)
{
	if (filebno < NDIRECT) {
        // block can be found directly.
		*ppdiskbno = &f->f_direct[filebno];
        return 0;
    } else if (filebno < NDIRECT + NINDIRECT){
		// block can be found indirectly.
		if (!f->f_indirect) {
			if (!alloc)
				return -E_NOT_FOUND;

			// allocate indirect block (* not file data block *)
			f->f_indirect = alloc_block();
			if (!f->f_indirect)
				return -E_NO_DISK;

            // clear block.
            memset(diskaddr(f->f_indirect), 0, BLKSIZE);
        }

        uint32_t *ind_blk = (uint32_t *)diskaddr(f->f_indirect);
        *ppdiskbno = &ind_blk[filebno - NDIRECT];
        return 0;
	} else {
		return -E_INVAL;
	}	
}

/* Set *blk to the address in memory where the filebno'th
 * block of file 'f' would be mapped.
 *
 * Returns 0 on success, < 0 on error. Errors are:
 *	-E_NO_DISK if a block needed to be allocated but the disk is full.
 *	-E_INVAL if filebno is out of range.
 *
 * Hint: Use file_block_walk and alloc_block.
 */
int file_get_block(File *f, uint32_t filebno, char **blk)
{
	// Find block's disk block number.
    uint32_t *pdiskbno;
	int r;

    if ((r = file_block_walk(f, filebno, &pdiskbno, 1)) < 0)
        return r;

    // Although file_block_walk would allocate indirect block
	// but file data block is not allocated, here allocate
	// the data block.
    if (!*pdiskbno)
        *pdiskbno = alloc_block();

    // Allocation failed.
    if (!*pdiskbno)
        return -E_NO_DISK;

    *blk = (char *)diskaddr(*pdiskbno);
    return 0;
}

/* Try to find a file named "name" in dir. If so, set *file to it.
 * 
 * Returns 0 and sets *file on success, < 0 on error.
 * Errors are: -E_NOT_FOUND if the file is not found.
 */
static int
dir_lookup(File *dir, const char *name, File **file)
{
	int r;
	uint32_t i, j, nblock;
	char *blk;
	File *f;

	// Search dir for name.
	// We maintain the invariant that the size of a directory-file
	// is always a multiple of the file system's block size.
	assert((dir->f_size % BLKSIZE) == 0);
	nblock = dir->f_size / BLKSIZE;

	for (i = 0; i < nblock; i++) {
		if ((r = file_get_block(dir, i, &blk)) < 0)
			return r;

		f = (File *) blk;
		for (j = 0; j < BLKFILES; j++) {
			if (strcmp(f[j].f_name, name) == 0) {
				*file = &f[j];
				return 0;
			}
		}
	}
	return -E_NOT_FOUND;
}

/* Evalate a path name, starting at the root.
 * On success, set *pf to the file we found and set *pdir to the directory
 * the file is in. If we cannot find the file but find the directory it
 * should be in, set *pdir and copy the final path element into lastelem.
 */
static int
walk_path(const char *path, File **pdir, File **pf, char *lastelem)
{
	const char *p;
	char name[MAXNAMELEN];
	File *dir, *f;
	int r;

	path = skip_slash(path);
	f = &super->s_root;
	dir = 0;
	name[0] = 0;

	if (pdir)
		*pdir = 0;
	*pf = 0;

	while (*path != '\0') {
		dir = f;
		p = path;

		while (*path != '/' && *path != '\0')
			path++;

		if (path - p >= MAXNAMELEN)
			return -E_BAD_PATH;

		memmove(name, p, path - p);
		name[path - p] = '\0';
		path = skip_slash(path);

		if (dir->f_type != FTYPE_DIR)
			return -E_NOT_FOUND;

		if ((r = dir_lookup(dir, name, &f)) < 0) {
			if ( r == -E_NOT_FOUND && *path == '\0') {
				if (pdir)
					*pdir = dir;
				if (lastelem)
					strcpy(lastelem, name);
				*pf = 0;
			}
			return r;
		}
	}
	if (pdir)
		*pdir = dir;
	*pf = f;
	return 0;
}

// --------------------------------------------------------------
// File operations
// --------------------------------------------------------------

/* Open "path". On success set *pf to point at the file and return 0.
 * On error return < 0.
 */
int
file_open(const char *path, File **pf)
{
	return walk_path(path, 0, pf, 0);
}
