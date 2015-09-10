#ifndef INCLUDE_FD_H
#define INCLUDE_FD_H

#include <include/types.h>
#include <include/fs.h>

struct Fd;
struct Stat;
struct Dev;

/* Per-device-class file descriptor operations */
struct Dev {
	int			dev_id;
	const char	*dev_name;
	ssize_t		(*dev_read)(struct Fd *fd, void *buf, size_t len);
	ssize_t		(*dev_write)(struct Fd *fd, const char *buf, size_t len);
	int			(*dev_close)(struct Fd *fd);
	int			(*dev_stat)(struct Fd *fd, struct Stat *stat);
	int			(*dev_trunc)(struct Fd *fd, off_t length);
};

struct FdFile {
	int			id;
};

struct Fd {
	int			fd_dev_id;
	off_t		fd_offset;
	int			fd_omode;
	union {
		// File server files
		struct FdFile	fd_file;
	};
};

struct Stat {
	char		st_name[MAXNAMELEN];
	off_t		st_size;
	int			st_isdir;
	struct Dev	*st_dev;
};

typedef struct FdFile FdFile;
typedef struct Fd Fd;
typedef struct Dev Dev;
typedef struct Stat Stat;
#endif
