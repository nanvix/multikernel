/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Must come first. */
#define __VFS_SERVER

#include <nanvix/config.h>
#include <nanvix/servers/vfs.h>
#include <nanvix/dev.h>
#include <nanvix/limits.h>
#include <posix/sys/types.h>
#include <posix/sys/stat.h>
#include <posix/errno.h>
#include <posix/fcntl.h>
#include <posix/unistd.h>

/**
 * @brief Table of Files
 */
static struct file filetab[NANVIX_NR_FILES];

/**
 * @brief Initializes a file.
 */
#define FILE_INITIALIZER(x) \
{                           \
	(x)->oflag = 0;         \
	(x)->count = 0;         \
	(x)->pos = 0;           \
	(x)->inode = NULL;      \
}

/*============================================================================*
 * getfile()                                                                  *
 *============================================================================*/

/**
 * The getfile() function searches the table of files for a free entry.
 */
struct file *getfile(void)
{
	struct file *f;

	/* Look for empty file table entry. */
	for (f = &filetab[0]; f < &filetab[NANVIX_NR_FILES]; f++)
	{
		/* Found. */
		if (f->count == 0)
			return (f);
	}

	return (NULL);
}

/*============================================================================*
 * getfildes()                                                                *
 *============================================================================*/

/**
 * @brief Gets a file descriptor.
 *
 * @returns Upon successful completion, a free file descriptor is
 * returned. Upon failure, a negative error code is returned instead.
 */
static int getfildes(void)
{
	/* Look for empty file descriptor table entry. */
	for (int i = 0; i < NANVIX_OPEN_MAX; i++)
	{
		/* Found. */
		if (curr_proc->ofiles[i] == NULL)
			return (i);
	}

	return (-EMFILE);
}

/*============================================================================*
 * fs_open()                                                                  *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
static struct inode *do_creat(
	struct inode *d,
	const char *name,
	mode_t mode,
	int oflag
)
{
	((void) d);
	((void) name);
	((void) mode);
	((void) oflag);

	return (NULL);
}

/**
 * @todo TODO: Provide a detailed description for this function.
 */
static struct inode *do_open(const char *filename, int oflag, mode_t mode)
{
	struct inode *dinode;   /* Directory's inode.       */
	off_t off;              /* Offset of Target Inode   */
	struct inode *i;        /* Inode of the Target File */
	struct d_dirent dirent; /* Directory Entry          */

	dinode = curr_proc->root;

	/* Failed to get directory. */
	if (dinode == NULL)
		return (NULL);

	/* Search file. */
	if ((off = minix_dirent_search(&dinode->data, filename, 0)) < 0)
	{
		/* Create it. */
		if ((i = do_creat(dinode, filename, mode, oflag)) == NULL)
			return (NULL);

		return (i);
	}

	/* Read Directory entry */
	if (bdev_read(minix_fs.dev, (char *) &dirent, sizeof(struct d_dirent), off) < 0)
		return (NULL);

	/* Read inode. */
	if ((i = inode_read(dirent.d_ino)) == NULL)
		return (NULL);

	/* Block special file. */
	if (S_ISBLK(i->data.i_mode))
	{
		if (bdev_open(i->data.i_zones[0]) < 0)
			goto error;
	}

	/* Regular file. */
	else if (S_ISREG(i->data.i_mode))
	{
		curr_proc->errcode = -ENOTSUP;
		goto error;
	}

	/* Directory. */
	else if (S_ISDIR(i->data.i_mode))
	{
		curr_proc->errcode = -ENOTSUP;
		goto error;
	}

	return (i);

error:
	ufree(i);
	return (NULL);
}

/**
 * The fs_open() function opens the file named @p filename. The @p
 * oflag parameter is used to set the opening flags, and @p mode is used
 * to defined the access mode for the file, if the @p O_CREAT flag is
 * passed.
 */
int fs_open(const char *filename, int oflag, mode_t mode)
{
	int fd;           /* File Descriptor  */
	struct file *f;   /* File             */
	struct inode *i;  /* Underlying Inode */

	/* Get a free file descriptor. */
	if ((fd = getfildes()) < 0)
		return (-EMFILE);

	/* Grab a free entry in the file table. */
	if ((f = getfile()) == NULL)
		return (-ENFILE);

	/* Increment reference count before actually opening
	 * the file because we can sleep below and another process
	 * may want to use this file table entry also.  */
	f->count = 1;

	/* Open file. */
	if ((i = do_open(filename, oflag, mode)) == NULL)
	{
		f->count = 0;
		return (curr_proc->errcode);
	}

	/* Initialize file. */
	f->oflag = oflag;
	f->pos = 0;
	f->inode = i;

	curr_proc->ofiles[fd] = f;

	return (fd);
}

/*============================================================================*
 * fs_close()                                                                 *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int fs_close(int fd)
{
	struct file *f;   /* Target File      */
	struct inode *ip; /* Underlying Inode */

	/* Bad file descriptor. */
	if ((f = curr_proc->ofiles[fd]) == NULL)
		return (-ENFILE);

	curr_proc->ofiles[fd] = NULL;

	/* File is opened by others. */
	if (f->count-- > 1)
		return (0);

	ip = f->inode;

	/* Block special file. */
	if (S_ISBLK(ip->data.i_mode))
	{
		if (bdev_close(ip->data.i_zones[0]) < 0)
			return (curr_proc->errcode);
	}

	/* Regular file. */
	else if (S_ISREG(ip->data.i_mode))
		return (curr_proc->errcode = -ENOTSUP);

	/* Directory. */
	else if (S_ISDIR(ip->data.i_mode))
		return (curr_proc->errcode = -ENOTSUP);

	/* Unknown file type. */
	else
		return (curr_proc->errcode = -ENOTSUP);

	return (inode_free(ip));
}

/*============================================================================*
 * fs_read()                                                                  *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
ssize_t fs_read(int fd, void *buf, size_t n)
{
	dev_t dev;        /* Device number.       */
	struct file *f;   /* File.                */
	struct inode *ip; /* Inode.               */
	ssize_t count;    /* Bytes actually read. */

	/* Bad file descriptor. */
	if ((f = curr_proc->ofiles[fd]) == NULL)
		return (-ENFILE);

	/* File not opened for reading. */
	if (ACCMODE(f->oflag) == O_WRONLY)
		return (-EBADF);

	/* Nothing to do. */
	if (n == 0)
		return (0);

	 ip = f->inode;

	/* Block special file. */
	if (S_ISBLK(ip->data.i_mode))
	{
		dev = ip->data.i_zones[0];
		count = bdev_read(dev, buf, n, f->pos);
	}

	/* Regular file/directory. */
	else if ((S_ISDIR(ip->data.i_mode)) || (S_ISREG(ip->data.i_mode)))
		count = file_read(ip, buf, n, f->pos);

	/* Unknown file type. */
	else
		return (-ENOTSUP);

	/* Failed to read. */
	if (count < 0)
		return (curr_proc->errcode);

	f->pos += count;

	return (count);
}

/*============================================================================*
 * fs_write()                                                                 *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
ssize_t fs_write(int fd, void *buf, size_t n)
{
	dev_t dev;         /* Device number.          */
	struct file *f;    /* File.                   */
	struct inode *ip;  /* Inode.                  */
	ssize_t count = 0; /* Bytes actually written. */

	/* Bad file descriptor. */
	if ((f = curr_proc->ofiles[fd]) == NULL)
		return (-ENFILE);

	/* File not opened for writing. */
	if (ACCMODE(f->oflag) == O_RDONLY)
		return (-EBADF);

	/* Nothing to do. */
	if (n == 0)
		return (0);

	ip = f->inode;

	/* Append mode. */
	if (f->oflag & O_APPEND)
		f->pos = ip->data.i_size;

	/* Block special file. */
	if (S_ISBLK(ip->data.i_mode))
	{
		dev = ip->data.i_zones[0];
		count = bdev_write(dev, buf, n, f->pos);
	}

	/* Regular file. */
	else if (S_ISREG(ip->data.i_mode))
		count = file_write(ip, buf, n, f->pos);

	/* Failed to write. */
	if (count < 0)
		return (curr_proc->errcode);

	f->pos += count;

	return (count);
}

/*============================================================================*
 * fs_lseek()                                                                 *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
off_t fs_lseek(int fd, off_t offset, int whence)
{
	off_t tmp;      /* Auxiliary variable. */
	struct file *f; /* File.               */

	/* Bad file descriptor. */
	if ((f = curr_proc->ofiles[fd]) == NULL)
		return (-ENFILE);

	/* Pipe file. */
	if (S_ISFIFO(f->inode->data.i_mode))
		return (-ESPIPE);
	
	/* Move read/write file offset. */
	switch (whence)
	{
		case SEEK_CUR :
			/* Invalid offset. */
			if (f->pos + offset < 0)
				return (-EINVAL);
			f->pos += offset;
			break;
		
		case SEEK_END :
			/* Invalid offset. */
			if ((tmp = f->inode->data.i_size + offset) < 0)
				return (-EINVAL);
			f->pos = tmp;
			break;
		
		case SEEK_SET :
			/* Invalid offset. */
			if (offset < 0)
				return (-EINVAL);
			f->pos = offset;
			break;
		
		default :
			return (-EINVAL);
	}
	
	return (f->pos);
}

/*============================================================================*
 * fs_init()                                                                  *
 *============================================================================*/

/**
 * The fs_init() function initializes the file system. It brings the
 * underlying devices up, formats the file system and initialize the
 * table of tiles.
 */
void fs_init(void)
{
	ramdisk_init();
	binit();
	minix_mkfs(
		NANVIX_ROOT_DEV,
		NR_INODES,
		NANVIX_DISK_SIZE/NANVIX_FS_BLOCK_SIZE,
		NANVIX_ROOT_UID,
		NANVIX_ROOT_GID
	);

	/* Initialize table of files. */
	for (int i = 0; i < NANVIX_NR_FILES; i++)
		FILE_INITIALIZER(&filetab[i]);
}
