#ifndef INCLUDE_FD_H
#define INCLUDE_FD_H

/* Per-device-class file descriptor operations */
typedef struct {
	int			dev_id;
	const char	*dev_name;
	ssize_t		(*dev_read)(Fd *fd, void *buf, size_t len);
	ssize_t		(*dev_write)(Fd *fd, const char *buf, size_t len);
	int			(*dev_close)(Fd *fd);
	int			(*dev_stat)(Fd *fd, Stat *stat);
	int			(*dev_trunc)(Fd *fd, off_t length);
} Dev;

typedef struct {
	int			id;
} FdFile;

typedef struct {
	int			fd_dev_id;
	off_t		fd_offset;
	int			fd_omode;
	union {
		// File server files
		FdFile	fd_file;
	};
} Fd;

typedef struct {
	char		st_name[MAXNAMELEN];
	off_t		st_size;
	int			st_isdir;
	Dev			*st_dev;
} Stat;


#endif
