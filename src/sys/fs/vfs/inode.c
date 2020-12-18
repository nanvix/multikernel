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

#include <nanvix/servers/vfs.h>
#include <nanvix/types/vfs.h>
#include <nanvix/sys/perf.h>
#include <nanvix/config.h>
#include <nanvix/dev.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

/**
 * @brief Table of Inodes
 */
static struct inode inodes[NANVIX_INODES_TABLE_LENGTH];

/**
 * @brief Pool of Inodes
 */
static struct resource_pool pool = {
	.resources = inodes,
	.nresources = NANVIX_INODES_TABLE_LENGTH,
	.resource_size = sizeof(struct inode)
};

/*============================================================================*
 * inode_disk_get()                                                           *
 *============================================================================*/

/**
 * The inode_disk_get() function gets the reference for the underlying
 * disk inode.
 */
struct d_inode *inode_disk_get(struct inode *ip)
{
	/* Invalid inode. */
	if (ip == NULL)
		return (NULL);

	/* Bad inode. */
	if (ip->count == 0)
		return (NULL);

	return (&ip->data);
}

/*============================================================================*
 * inode_get_num()                                                            *
 *============================================================================*/

/**
 * The inode_get_num() function gets the number of the inode pointed to
 * by @p ip.
 */
ino_t inode_get_num(const struct inode *ip)
{
	/* Invalid inode. */
	if (ip == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (MINIX_INODE_NULL);
	}

	/* Bad inode. */
	if (ip->count == 0)
	{
		curr_proc->errcode = -EINVAL;
		return (MINIX_INODE_NULL);
	}

	return (ip->num);
}


/*============================================================================*
 * inode_null()                                                           *
 *============================================================================*/

/**
 * The inode_null() function zeroes the number of the inode pointed to
 * by @p ip.
 */
void inode_null(struct inode *ip)
{
	/* Invalid inode. */
	if (ip == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return;
	}

	/* Bad inode. */
	if (ip->count == 0)
	{
		curr_proc->errcode = -EINVAL;
		return;
	}

	ip->num = MINIX_INODE_NULL;
}

/*============================================================================*
 * inode_get_count()                                                          *
 *============================================================================*/

/**
 * The inode_get_count() function gets the inode count of the inode pointed to
 * by @p ip.
 */
int inode_get_count(const struct inode *ip)
{
	/* Invalid inode. */
	if (ip == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (MINIX_INODE_NULL);
	}

	return (ip->count);
}

/*============================================================================*
 * inode_set_count()                                                          *
 *============================================================================*/

/**
 * The inode_set_count() function sets the inode count of the inode pointed to
 * by @p ip to the value of @p c.
 */
int inode_set_count(struct inode *ip, const int c)
{
	/* Invalid inode. */
	if (ip == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (MINIX_INODE_NULL);
	}

	ip->count = c;

	return (0);
}

/**
 * The inode_increase_count() function increases the inode count of the inode pointed to
 * by @p ip by 1
 */
int inode_increase_count(struct inode *ip)
{
	/* Invalid inode. */
	if (ip == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (MINIX_INODE_NULL);
	}

	ip->count++;

	return (0);
}

/**
 * The inode_decrease_count() function decreases the inode count of the inode pointed to
 * by @p ip by 1
 */
int inode_decrease_count(struct inode *ip)
{
	/* Invalid inode. */
	if (ip == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (MINIX_INODE_NULL);
	}

	ip->count--;

	return (0);
}

/*============================================================================*
 * inode_get_dev()                                                            *
 *============================================================================*/

/**
 * The inode_get_dev() function gets the device number of the inode
 * pointed to by @p ip.
 */
dev_t inode_get_dev(const struct inode *ip)
{
	/* Invalid inode. */
	if (ip == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (NANVIX_DEV_NULL);
	}

	/* Bad inode. */
	if (ip->count == 0)
	{
		curr_proc->errcode = -EINVAL;
		return (NANVIX_DEV_NULL);
	}

	return (ip->dev);
}

/*============================================================================*
 * inode_set_dirty()                                                          *
 *============================================================================*/

/**
 * The inode_set_dirty() sets the inode pointed to by @p ip as dirty.
 */
int inode_set_dirty(struct inode *ip)
{
	int idx;

	/* Invalid inode. */
	if (ip == NULL)
		return (curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (ip->count == 0)
		return (curr_proc->errcode = -EINVAL);

	idx = ip - inodes;

	/* Bad inode. */
	if (!WITHIN(idx, 0, NANVIX_INODES_TABLE_LENGTH))
		return (curr_proc->errcode = -EINVAL);

	resource_set_dirty(&ip->resource);

	return (0);
}

/*============================================================================*
 * inode_read()                                                               *
 *============================================================================*/

/**
 * @brief Reads an inode to memory.
 *
 * @param fs   Target file system.
 * @param num Number of the target inode.
 *
 * @returns Upon successful completion, a pointer to the target inode is
 * returned. Upon failure, a negative error code is returned instead.
 */
static struct inode *inode_read(struct filesystem *fs, ino_t num)
{
	int idx;          /* inode index  */
	struct inode *ip; /* Inode        */

	/* Invalid file system. */
	if (fs == NULL)
		return (NULL);

	/* Allocate memory inode. */
	if ((idx = resource_alloc(&pool)) < 0)
	{
		uprintf("[nanvix][vfs] inodes table overflow");
		curr_proc->errcode = -ENOMEM;
		goto error0;
	}

	ip = &inodes[idx];

	/* Read disk inode. */
	if (minix_inode_read(fs->dev, &fs->super->data, &ip->data, num) < 0)
		goto error1;

	/* Initialize inode. */
	ip->count = 1;
	ip->num = num;
	ip->dev = fs->dev;

	return (ip);

error1:
	resource_free(&pool, idx);
error0:
	uassert(minix_inode_free(&fs->super->data, fs->super->imap, num) == 0);
	return (NULL);
}

/*============================================================================*
 * inode_touch()                                                              *
 *============================================================================*/

/**
 * The inode_touch() function updates the time stamp of the inode
 * pointed to by @p ip.
 */
int inode_touch(struct inode *ip)
{
	uint64_t now;

	/* Invalid inode. */
	if (ip == NULL)
		return (curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (ip->count == 0)
		return (curr_proc->errcode = -EINVAL);

	kclock(&now);
	ip->data.i_time = now;

	return (0);
}

/*============================================================================*
 * inode_free()                                                               *
 *============================================================================*/

/**
 * @brief Releases an in-memory inode.
 *
 * @param fs   Target file system.
 * @param ip Target inode.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int inode_free(struct filesystem *fs, struct inode *ip)
{
	int idx;

	idx = ip - inodes;

	/* Bad inode. */
	if (!WITHIN(idx, 0, NANVIX_INODES_TABLE_LENGTH))
		return (curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (ip->count == 0)
		return (curr_proc->errcode = -EBUSY);

	/* Release inode. */
	if (ip->count-- == 1)
	{
		if (ip->data.i_nlinks-- == 1)
		{
			if (minix_inode_free(&fs->super->data, fs->super->imap, ip->num) < 0)
			{
				uprintf("[nanvix][vfs] failed to release inode %d", ip->num);
				return (curr_proc->errcode = -EAGAIN);
			}
		}

		/* House keeping. */
		resource_free(&pool, idx);
	}

	return (0);
}

/*============================================================================*
 * inode_put()                                                                *
 *============================================================================*/

/**
 * @brief Releases the reference to an inode.
 *
 * @param fs Target file system.
 * @param ip Target inode.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int inode_put(struct filesystem *fs, struct inode *ip)
{
	int idx;

	/* Invalid file system */
	if (fs == NULL)
		return (curr_proc->errcode = -EINVAL);

	/* Invalid inode. */
	if (ip == NULL)
		return (curr_proc->errcode = -EINVAL);

	idx = ip - inodes;

	/* Bad inode. */
	if (!WITHIN(idx, 0, NANVIX_INODES_TABLE_LENGTH))
		return (curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (fs->dev != ip->dev)
		return (curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (ip->count == 0)
		return (curr_proc->errcode = -EINVAL);

	/* Write inode back to disk. */
	if (minix_inode_write(ip->dev, &fs->super->data, &ip->data, ip->num) < 0)
	{
		uprintf("[nanvix][vfs] failed to write inode %d", ip->num);
		return (curr_proc->errcode = -EAGAIN);
	}

	return (inode_free(fs, ip));
}

/*============================================================================*
 * inode_write()                                                              *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int inode_write(struct filesystem *fs, struct inode *ip)
{
	/* Invalid file system */
	if (fs == NULL)
		return (curr_proc->errcode = -EINVAL);

	/* Invalid inode. */
	if (ip == NULL)
		return (curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (fs->dev != ip->dev)
		return (curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (ip->count == 0)
		return (curr_proc->errcode = -EINVAL);

	/* Write disk inode. */
	if (minix_inode_write(ip->dev, &fs->super->data, &ip->data, ip->num) < 0)
	{
		uprintf("[nanvix][vfs] failed to write inode %d", ip->num);
		return (curr_proc->errcode = -EAGAIN);
	}

	return (0);
}

/*============================================================================*
 * inode_get()                                                                *
 *============================================================================*/

/**
 * The inode_get() function gets a reference to the inode specified by
 * @p num that resides in the file system pointed to by @p fs.
 */
struct inode *inode_get(struct filesystem *fs, ino_t num)
{
	/* Invalid file system. */
	if (fs == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (NULL);
	}

	/* Invalid inode number. */
	if (num >= NANVIX_NR_INODES)
	{
		curr_proc->errcode = -EINVAL;
		return (NULL);
	}

	/* Search for inode in the table of inodes. */
	for (int i = 0; i < NANVIX_INODES_TABLE_LENGTH; i++)
	{
		/* Skip invalid entries. */
		if (!resource_is_used(&inodes[i].resource))
			continue;

		/* Found. */
		if ((inodes[i].dev == fs->dev) && (inodes[i].num == num))
		{
			inodes[i].count++;
			return (&inodes[i]);
		}
	}

	/* Read inode in. */
	return (inode_read(fs, num));
}

/*============================================================================*
 * inode_alloc()                                                              *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
struct inode *inode_alloc(
	struct filesystem *fs,
	mode_t mode,
	uid_t uid,
	gid_t gid
)
{
	ino_t num;

	/* Invalid file system */
	if (fs == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (NULL);
	}

	/* Bad file system */
	if (fs->root == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (NULL);
	}

	/* Allocate disk inode. */
	if ((num = minix_inode_alloc(fs->dev, &fs->super->data, fs->super->imap, mode, uid, gid)) == MINIX_INODE_NULL)
	{
		curr_proc->errcode = -EAGAIN;
		return (NULL);
	}

	return (inode_read(fs, num));
}

/*============================================================================*
 * inode_name()                                                               *
 *============================================================================*/

/**
 * @brief Gets the first path part and stores it in @p dirname
 */
char *dirname(const char *path, char *dirname)
{
	const char *pa;    /* path pointer    */
	char *dp;          /* dirname pointer */

	if (path == NULL)
		return (NULL);

	pa = path;
	dp = dirname;

	/* ignore beggining '/' */
	while (*pa == '/')
		pa++;

	/* read until next '/' or end of string */
	while (*pa != '/' && *pa != '\0') {

		/* name too long */
		if ((dp - dirname) > NANVIX_NAME_MAX)
			return (NULL);

		*dp++ = *pa++;
	}

	*dp = '\0';

	return dirname;
}

/**
 * The inode_name() function lookups an inode by the name pointed to by
 * @p name. The root directory of @p fs is used as start point for the
 * search.
 *
 * @todo TODO: recursive lookup.
 * @todo TODO: support relative path search.
 */
struct inode *inode_name(struct filesystem *fs, const char *name)
{
	struct inode *dinode;   /* Directory's inode.       */
	off_t off;              /* Offset of Target Inode   */
	struct d_dirent dirent; /* Directory Entry          */
	const char *remainder;  /* Path remainder string    */
	char *needle;           /* searched directory entry */
	char filename[NANVIX_NAME_MAX + 1];

	remainder = name;

	/* Invalid file system */
	if (fs == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (NULL);
	}

	/* Invalid name. */
	if (name == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (NULL);
	}

	/* root directory */
	if ((ustrncmp(name, "/", NANVIX_NAME_MAX)) == 0)
		return inode_get(fs, MINIX_INODE_ROOT);

	/* absolute path */
	if (*remainder == '/')
		dinode = inode_get(fs, MINIX_INODE_ROOT);

	/* relative path */
	else
		dinode = inode_get(fs, inode_get_num(curr_proc->pwd));

	/* parse path */
	while (*remainder != '\0') {
		/* Failed to get directory. */
		if (dinode == NULL)
		{
			curr_proc->errcode = -EINVAL;
			return (NULL);
		}

		/* TODO: Use real uid and gid */
		if (!(has_permissions(
						inode_disk_get(dinode)->i_mode,
						NANVIX_ROOT_UID,
						NANVIX_ROOT_GID,
						(S_IRUSR | S_IRGRP | S_IROTH))
					)) {
			curr_proc->errcode = -EACCES;
			return (NULL);
		}

		/* skip beggining '/' */
		while (*remainder == '/')
			remainder++;

		/* get the dirname of the path */
		if ((needle = dirname(remainder, filename)) == NULL)
			return (NULL);

		/* Search file. */
		if ((off = minix_dirent_search(fs->dev, &fs->super->data, fs->super->bmap, inode_disk_get(dinode), needle, 0)) < 0)
		{
			curr_proc->errcode = -ENOENT;
			return (NULL);
		}

		/* Read Directory entry */
		if (bdev_read(fs->dev, (char *) &dirent, sizeof(struct d_dirent), off) < 0)
		{
			curr_proc->errcode = -EIO;
			return (NULL);
		}

		dinode = inode_get(&fs_root, dirent.d_ino);
		/* move to the next directory in path */
		while (*remainder != '/' && *remainder != '\0')
			remainder++;
	}

	return (inode_get(&fs_root, dirent.d_ino));
}

/*============================================================================*
 * inode_init()                                                               *
 *============================================================================*/

/**
 * The inode_init() function initializes the inodes module.
 */
void inode_init(void)
{
	for (int i = 0; i < NANVIX_INODES_TABLE_LENGTH; i++)
	{
		inodes[i].resource = RESOURCE_INITIALIZER;
		inodes[i].dev = -1;
		inodes[i].num = MINIX_INODE_NULL;
		inodes[i].count = 0;
	}
}
