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
#include <nanvix/servers/vfs/bcache.h>
#include <nanvix/servers/vfs/inode.h>
#include <nanvix/types/vfs.h>
#include <nanvix/dev.h>
#include <nanvix/limits.h>
#include <posix/sys/types.h>
#include <posix/errno.h>
#include <posix/fcntl.h>
#include <posix/unistd.h>
#include <posix/stdlib.h>

/**
 * Root file system.
 */
struct filesystem fs_root;

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


/**
 * @brief Checks file access permissions
 * TODO use real uid and gid instead of superuser
 */
mode_t has_permissions(mode_t mode, nanvix_uid_t uid, nanvix_gid_t gid, mode_t mask)
{
	mode &= mask;

	/* Super user */
	if (IS_SUPERUSER(NANVIX_ROOT_UID))
		mode &= (S_IRWXU | S_IRWXG | S_IRWXO);

	/* owner user */
	else if (NANVIX_ROOT_UID == uid)
		mode &= (S_IRUSR | S_IWUSR | S_IXUSR);

	/* group */
	else if (NANVIX_ROOT_GID == gid)
		mode &= (S_IRGRP | S_IWGRP | S_IXGRP);

	/* Others */
	else
		mode &= (S_IRWXO);

	return (mode);
}


/*============================================================================*
 * getfile()                                                                  *
 *============================================================================*/

/**
 * @brief Gets an empty file.
 *
 * The getfile() function searches the table of files for a free entry.
 *
 * @returns Upon successful completion, a pointer to an empty file is
 * returned. Upon failure, a NULL pointer is returned instead.
 */
static struct file *getfile(void)
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
 * fs_truncate()                                                                  *
 *============================================================================*/

/**
 * @brief This function trucates the inode @p ip
 * that exists under the filesystem @p fs
 *
 * @param fs: filesystem
 * @param ip: inode to be truncated
 *
 * @author Lucca Augusto
 */
void do_trucate(struct filesystem *fs, struct inode *ip)
{
	struct d_inode *ino_data;     /* underlying d_inode */

	ino_data = inode_disk_get(ip);

	/* TODO: update timestamps */

	/* Free direct zone. */
	for (int j = 0; j < MINIX_NR_ZONES_DIRECT; j++)
	{
		minix_block_free_direct(&(fs->super->data), fs->super->bmap, j);
		ino_data->i_zones[j] = MINIX_BLOCK_NULL;
	}

	/* Free singly indirect zones. */
	for (int j = 0; j < MINIX_NR_ZONES_SINGLE; j++)
	{
		minix_block_free_indirect(&(fs->super->data), fs->super->bmap, j);
		ino_data->i_zones[MINIX_ZONE_SINGLE + j] = MINIX_BLOCK_NULL;
	}

	/* Free double indirect zones. */
	for (int j = 0; j < MINIX_NR_ZONES_DOUBLE; j++)
	{
		minix_block_free_dindirect(&(fs->super->data), fs->super->bmap, j);
		ino_data->i_zones[MINIX_ZONE_DOUBLE + j] = MINIX_BLOCK_NULL;
	}

	ino_data->i_size = 0;
	inode_touch(ip);
}


/**
 * @brief See do_trucate
 */
int fs_trucate(struct filesystem *fs, struct inode *ip)
{

	/* Invalid file system */
	if (fs == NULL)
		return (curr_proc->errcode = -EINVAL);

	/* Invalid inode. */
	if (ip == NULL)
		return (curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (fs->dev != inode_get_dev(ip))
		return (curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (inode_get_count(ip) == 0)
		return (curr_proc->errcode = -EINVAL);

	/* truncate inode. */
	do_trucate(fs, ip);

	return (0);
}

/*============================================================================*
 * fs_open()                                                                  *
 *============================================================================*/

/**
 * @brief: Creates a file
 *
 * @param name: name of the file to be created
 * @param mode: access mode flags
 * @param oflag: creation flags
 *
 * @returns Upon successful completion an inode to the newly created file
 * is returned. Upon failure, NULL is returned instead
 */
static struct inode *do_creat( const char *name, int oflag, mode_t mode)
{
	int exists = 0;           /* file already exists   */
	int dadd_err = 0;         /* dirent add error code */
	struct inode *ip;         /* inode                 */
	struct inode *curr_dir;   /* current directory     */

	/* not asked to create file or no permissions to write*/
	if (!(oflag & O_CREAT)) {
		curr_proc->errcode = -(ENOENT);
		return (NULL);
	}

	if (mode <= 0) {
		curr_proc->errcode = -(EINVAL);
		return (NULL);
	}

	mode &= ~(curr_proc->umask);
	mode |= S_IFREG;

	/* file already exists */
	if ((ip = inode_name(&fs_root, name)) != NULL) {
		exists = 1;
		/* no write permitions */
		/* TODO: use real uid and gid */
		if (!(has_permissions(mode, NANVIX_ROOT_UID, NANVIX_ROOT_GID, (S_IWUSR | S_IWGRP | S_IWOTH)))) {
			curr_proc->errcode = -(EACCES);
			goto error;
		}

		/* directory */
		if (S_ISDIR(inode_disk_get(ip)->i_mode)) {
			curr_proc->errcode = -(EINVAL);
			goto error;
		}
	}

	/* file does not exist */
	else {
		/* TODO: find current directory */
		curr_dir = curr_proc->pwd;

		/* no write permissions to the directory */
		/* TODO: use real uid and gid */
		if (!(has_permissions(inode_disk_get(curr_dir)->i_mode,
						NANVIX_ROOT_UID,
						NANVIX_ROOT_GID,
						(S_IWUSR | S_IWGRP | S_IWOTH))
			 ))
		{
			curr_proc->errcode = -(EACCES);
			return (NULL);
		}

		/* alocate inode and create dirent */

		/* TODO: pass uid and gid instead of superuser */
		if ((ip = inode_alloc(&fs_root, mode, NANVIX_ROOT_UID, NANVIX_ROOT_GID)) == NULL)
			/* errcode set by inode_alloc */
			return (NULL);

		dadd_err = minix_dirent_add( inode_get_dev(ip),
						&(fs_root.super->data),
						fs_root.super->bmap,
						inode_disk_get(curr_dir),
						name,
						inode_get_num(ip)
					);

		if (dadd_err < 0) {
			curr_proc->errcode = (dadd_err);
			goto error;
		}

	}

	/* file already existed, truncate it */
	if (exists && (oflag & O_TRUNC)) {
		if (fs_trucate(&fs_root, ip) != 0) {
			goto error;
		}
	}

	return ip;
error:
	inode_put(&fs_root, ip);
	return (NULL);

}

/**
 * @brief Opens a file
 *
 * @returns Upon successful completion, a inode is
 * returned. Upon failure, NULL is returned instead.
 */
static struct inode *do_open(const char *filename, int oflag, mode_t mode)
{
	struct inode *ip;

	/* Invalid filename. */
	if (filename == NULL || *filename == '\0')
	{
		curr_proc->errcode = -EINVAL;
		return (NULL);
	}

	/* Search file and create it if flag has O_CREAT bit. */
	if ((ip = inode_name(&fs_root, filename)) == NULL)
	{
		if (!(oflag & O_CREAT)) {
			curr_proc->errcode = -ENOENT;
			return (NULL);
		}

		if ((ip = do_creat(filename, oflag, mode)) == NULL)
			return (NULL);
		return (ip);
	}


	/* Block special file. */
	if (S_ISBLK(inode_disk_get(ip)->i_mode))
	{
		if (bdev_open(inode_disk_get(ip)->i_zones[0]) < 0)
			goto error;
	}

	/* Regular file. */
	else if (S_ISREG(inode_disk_get(ip)->i_mode))
	{
		curr_proc->errcode = -ENOTSUP;
		goto error;
	}

	/* Directory. */
	else if (S_ISDIR(inode_disk_get(ip)->i_mode))
	{
		curr_proc->errcode = -ENOTSUP;
		goto error;
	}

	return (ip);

error:
	inode_put(&fs_root, ip);
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
		f->count = -1;
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
 * fs_stat()                                                                  *
 *============================================================================*/

/**
 * @brief Counts number of blocks a file occupies
 *
 * @param ip File inode
 */
int file_block_count(struct inode *ip)
{
	int nr_blocks = 0;          /* block count                  */
	char *buf_data;             /* buffer data                  */
	char *buf_data_di;          /* buffer data indirect         */
	struct buffer *blk_buf;     /* block buffer                 */
	struct buffer *blk_buf_di;  /* block buffer double indirect */
	struct d_inode *ino_data;   /* inode data                   */

	/* invalid inode */
	if (ip == NULL) {
		return -EINVAL;
	}

 	ino_data = inode_disk_get(ip);

	/* Count number of blocks */
	for (unsigned int i=0; i < MINIX_NR_ZONES; ++i) {
		if (i == MINIX_ZONE_DOUBLE) {
			/* counting double indirect zones */

			blk_buf = bread(inode_get_dev(ip), ino_data->i_zones[i]);
			buf_data = buffer_get_data(blk_buf);

			/* traverse first indirect zone */
			for (unsigned j=0; j < MINIX_NR_DOUBLE; ++j) {

				/* count zones if block is not null */
				if (buf_data[j] != MINIX_BLOCK_NULL) {

					blk_buf_di = bread(inode_get_dev(ip),buf_data[j]);
					buf_data_di = buffer_get_data(blk_buf_di);
					/* count number of zones inside each indirect zone */

					/* traverse second indirect zone */
					for (unsigned k=0; k < MINIX_NR_SINGLE; ++k) {

						if (buf_data_di[k] != MINIX_BLOCK_NULL) {

							++nr_blocks;

						} else {
							/* quit all loops */
							i = MINIX_NR_ZONES;
							j = MINIX_NR_DOUBLE;
							break;
						}
					}
				} else {
					/* quit all loops */
					j = MINIX_NR_DOUBLE;
					break;
				}

			}

		} else if (i == MINIX_ZONE_SINGLE) {
			/* counting single indirect zones */

			/* count zones if block is not null */
			if (ino_data->i_zones[i] != MINIX_BLOCK_NULL) {

				blk_buf = bread(inode_get_dev(ip),ino_data->i_zones[i]);
				buf_data = buffer_get_data(blk_buf);

				for (unsigned j=0; j < MINIX_NR_SINGLE; ++j) {
					if (buf_data[j] != MINIX_BLOCK_NULL) {
						++nr_blocks;
					} else {
						/* quit both loops */
						i = MINIX_NR_ZONES;
						break;
					}
				}
			}

		} else if (ino_data->i_zones[i] != MINIX_BLOCK_NULL ) {
			/* counting direct zones */
			++nr_blocks;
		} else {
			/* found MINIX_BLOCK_NULL so last zone was counted */
			break;
		}
	}

	return nr_blocks;
}

/**
 * @brief Get stats about a file
 * The do_stat function retrieves information about the file @p filename
 * and writes it to the stat buffer @p buf.
 * @returns 0 in case it succeeds, a negative error code instead.
 */
static int do_stat(const char *filename, struct nanvix_stat *restrict buf)
{
	struct inode *ip;           /* file inode                   */
	struct d_inode *ino_data;   /* inode data                   */
	nanvix_dev_t rdev = 0;      /* dev id if special file       */

	/* Invalid filename. */
	if (filename == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return -EINVAL;
	}

	/* Search file. */
	if ((ip = inode_name(&fs_root, filename)) == NULL)
	{
		/* File doesn't exist*/
		return (curr_proc->errcode = -ENOENT);
	}

	ino_data = inode_disk_get(ip);

	/* Block special file. */
	if (S_ISBLK(ino_data->i_mode))
	{
		rdev = inode_get_dev(ip);
		if (bdev_open(ino_data->i_zones[0]) < 0)
			goto error;
	}

	/* Regular file. */
	else if (S_ISREG(ino_data->i_mode))
	{
		curr_proc->errcode = -ENOTSUP;
		goto error;
	}

	/* Directory. */
	else if (S_ISDIR(ino_data->i_mode))
	{
		curr_proc->errcode = -ENOTSUP;
		goto error;
	}

	/* file stats */
	/*TODO Update time related fields first */

	/* write stats in buf */
	buf->st_dev = inode_get_dev(ip);
	buf->st_ino = inode_get_num(ip);
	buf->st_mode = ino_data->i_mode;
	buf->st_nlink = ino_data->i_nlinks;
	buf->st_uid = ino_data->i_uid;
	buf->st_gid = ino_data->i_gid;
	buf->st_rdev = rdev;
	buf->st_size = ino_data->i_size;
	buf->st_blksize = NANVIX_FS_BLOCK_SIZE;
	buf->st_blocks = file_block_count(ip);

	inode_put(&fs_root, ip);
	return 0;

error:
	inode_put(&fs_root, ip);
	return curr_proc->errcode;
}

/**
 * The fs_stat() function returns information about the file
 * named @p filename.
 */
int fs_stat(const char *filename, struct nanvix_stat *restrict buf)
{
	int fd;           /* File Descriptor      */
	struct file *f;   /* File                 */

	/* Get a free file descriptor. */
	if ((fd = getfildes()) < 0)
		return -EMFILE;

	/* Grab a free entry in the file table. */
	if ((f = getfile()) == NULL)
		return -ENFILE;

	/* Increment reference count before actually opening
	 * the file because we can sleep below and another process
	 * may want to use this file table entry also.  */
	f->count = 1;

	/* Get file stat. */
	if (do_stat(filename, buf) != 0)
	{
		f->count = 0;
		return curr_proc->errcode;
	}
	return 0;
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
		return (curr_proc->errcode = -EBADF);

	curr_proc->ofiles[fd] = NULL;

	/* File is opened by others. */
	if (f->count-- > 1)
		return (0);

	ip = f->inode;

	/* Block special file. */
	if (S_ISBLK(inode_disk_get(ip)->i_mode))
	{
		if (bdev_close(inode_disk_get(ip)->i_zones[0]) < 0)
			return (curr_proc->errcode);
	}

	/* Regular file. */
	else if (S_ISREG(inode_disk_get(ip)->i_mode)) {
		/* Inode is used by others */
		if (inode_get_count(ip) > 1) {
			inode_decrease_count(ip);
			return (0);
		}
	}

	/* Directory. */
	else if (S_ISDIR(inode_disk_get(ip)->i_mode))
		return (curr_proc->errcode = -ENOTSUP);

	/* Unknown file type. */
	else
		return (curr_proc->errcode = -ENOTSUP);

	return (inode_put(&fs_root, ip));
}

/*============================================================================*
 * fs_unlink()                                                                *
 *============================================================================*/

int do_unlink(const char *filename, struct inode *fip)
{
	int ret = 0;       /* return value    */
	struct inode *dip; /* directory inode */

	/*TODO get parent directory inode */
	dip = inode_get(&fs_root, inode_get_num(curr_proc->pwd));

	/* remove from region table */
	ret = minix_dirent_remove(
			fs_root.dev,
			&(fs_root.super->data),
			fs_root.super->bmap,
			inode_disk_get(dip),
			filename
		);


	if (ret < 0)
		goto error;

	/* decrement file link count */
	inode_null(fip);
	inode_decrease_count(fip);

	inode_touch(dip);

	inode_put(&fs_root, dip);
	inode_put(&fs_root, fip);

	return (0);

error:
	inode_put(&fs_root, dip);
	inode_put(&fs_root, fip);

	return ret;
}

/**
 * @brief Unlink a file from it's directory
 *
 * @param filename: path of the file to be unlinked
 */
int fs_unlink(const char *filename)
{
	int ret = 0;            /* return value           */
	struct inode *fip;      /* file inode             */

	if (filename == NULL)
		return (-EINVAL);

	/* get file inode */
	if ((fip = inode_name(&fs_root, filename)) == NULL) {
		return (-ENOENT);
	}

	/* unlinking current directory */
	if (inode_get_num(fip) == inode_get_num(curr_proc->pwd)) {
		ret = (-EINVAL);
		goto error;
	}

	/* unlink directory */
	if (S_ISDIR(inode_disk_get(fip)->i_mode)) {

		/* TODO use real uid of process owner */
		if (!IS_SUPERUSER(NANVIX_ROOT_UID)) {
			ret = (-EACCES);
			goto error;
		}

		/* not empty */
		if (inode_disk_get(fip)->i_size) {
			ret = (-EBUSY);
			goto error;
		}
	}

	if ((ret = do_unlink(filename, fip)) != 0)
		goto error;

	inode_touch(fip);
	inode_put(&fs_root, fip);

	return (0);
error:
	inode_put(&fs_root, fip);
	return (ret);
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
		return (-EBADF);

	/* File not opened for reading. */
	if (ACCMODE(f->oflag) == O_WRONLY)
		return (-EBADF);

	/* Nothing to do. */
	if (n == 0)
		return (0);

	 ip = f->inode;

	/* Block special file. */
	if (S_ISBLK(inode_disk_get(ip)->i_mode))
	{
		dev = inode_disk_get(ip)->i_zones[0];
		count = bdev_read(dev, buf, n, f->pos);
	}

	/* Regular file/directory. */
	else if ((S_ISDIR(inode_disk_get(ip)->i_mode)) || (S_ISREG(inode_disk_get(ip)->i_mode)))
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
		return (-EBADF);

	/* File not opened for writing. */
	if (ACCMODE(f->oflag) == O_RDONLY)
		return (-EBADF);

	/* Nothing to do. */
	if (n == 0)
		return (0);

	ip = f->inode;

	/* Append mode. */
	if (f->oflag & O_APPEND)
		f->pos = inode_disk_get(ip)->i_size;

	/* Block special file. */
	if (S_ISBLK(inode_disk_get(ip)->i_mode))
	{
		dev = inode_disk_get(ip)->i_zones[0];
		count = bdev_write(dev, buf, n, f->pos);
	}

	/* Regular file. */
	else if (S_ISREG(inode_disk_get(ip)->i_mode))
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
		return (-EBADF);

	/* Pipe file. */
	if (S_ISFIFO(inode_disk_get(f->inode)->i_mode))
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
			if ((tmp = inode_disk_get(f->inode)->i_size + offset) < 0)
				return (-EINVAL);
			f->pos = tmp;
			break;

		case SEEK_SET :
			/* Invalid offset. */
			if (offset < 0)
				return (-EINVAL);
			f->pos = offset;
			break;

		default:
			return (-EINVAL);
	}

	return (f->pos);
}

/*============================================================================*
 * fs_make()                                                                  *
 *============================================================================*/

/**
 * The fs_make() function creates a file system on the device @p dev.
 * The file system is formatted to feature @p ninode inodes, @p nblocks
 * @p nblocks. Furthermore, the user ID and the user group ID of the
 * file system are set to @p uid, and @p gid, respectively.
 * @author Pedro Henrique Penna
 */
int fs_make(
	dev_t dev,
	ino_t ninodes,
	block_t nblocks,
	uid_t uid,
	gid_t gid
)
{
	return (
		minix_mkfs(
			dev,
			ninodes,
			nblocks,
			uid,
			gid
		)
	);
}

/*============================================================================*
 * fs_mount()                                                                 *
 *============================================================================*/

/**
 * The fs_mount() function mounts the file system that resides in the
 * device specified by @p dev. The information concerning the mounted
 * file system is stored in the location pointed to by @p fs.
 *
 * @author Pedro Henrique Penna
 */
int fs_mount(struct filesystem *fs, dev_t dev)
{
	/* Invalid argument. */
	if (fs == NULL)
		return (curr_proc->errcode = -EINVAL);

	/* Allocate memory for superblock */
	if ((fs->super = umalloc(sizeof(struct superblock))) == NULL)
		return (curr_proc->errcode = -ENOMEM);

	/* Mount file system. */
	uprintf("[nanvix][vfs][minix] mounting file system on device %d", dev);
	if (minix_mount(
			&fs->super->data,
			&fs->super->imap,
			&fs->super->bmap,
			fs->dev = dev
		) < 0
	)
		goto error0;

	/* Get reference root inode. */
	if ((fs->root = inode_get(fs, MINIX_INODE_ROOT)) == NULL)
	{
		curr_proc->errcode = -ENOMEM;
		goto error1;
	}

	/* double get on root to keep it available */
	if ((fs->root = inode_get(fs, MINIX_INODE_ROOT)) == NULL)
	{
		curr_proc->errcode = -ENOMEM;
		goto error1;
	}

	return (0);

error1:
	ufree(fs->super);
error0:
	return (curr_proc->errcode);
}

/*============================================================================*
 * fs_unmount()                                                               *
 *============================================================================*/

/**
 * The minix_unmount() function unmounts the file system pointed to by
 * @p fs. Any changes made to the file system structure are flushed back
 * to disk.
 *
 * @author Pedro Henrique Penna
 */
int fs_unmount(struct filesystem *fs)
{
	int err;

	/* Invalid argument. */
	if (fs == NULL)
		return (curr_proc->errcode = -EINVAL);

	/* Release root inode. */
	if ((err = inode_put(fs, fs->root)) < 0)
		return (curr_proc->errcode = err);

	/* Unmount file system. */
	uprintf("[nanvix][vfs][minix] unmounting file system on device %d", fs->dev);
	if ((err = minix_unmount(
			&fs->super->data,
			fs->super->imap,
			fs->super->bmap,
			fs->dev
		)) < 0
	)
		return (curr_proc->errcode = err);

	/* House keeping. */
	ufree(fs->super);

	return (0);
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

	/* Create root file system. */
	uassert(
		fs_make(
			NANVIX_ROOT_DEV,
			NANVIX_NR_INODES,
			NANVIX_DISK_SIZE/NANVIX_FS_BLOCK_SIZE,
			NANVIX_ROOT_UID,
			NANVIX_ROOT_GID
		) == 0
	);

	/* Mount root file system. */
	uassert(fs_mount(&fs_root, NANVIX_ROOT_DEV) == 0);

	/* Initialize table of files. */
	for (int i = 0; i < NANVIX_NR_FILES; i++)
		FILE_INITIALIZER(&filetab[i]);
}

/*============================================================================*
 * fs_shutdown()                                                              *
 *============================================================================*/

/**
 * The fs_shutdown() function shutdowns the file system.
 */
void fs_shutdown(void)
{
	/* Unmount root file system. */
	uassert(fs_unmount(&fs_root) == 0);
}
