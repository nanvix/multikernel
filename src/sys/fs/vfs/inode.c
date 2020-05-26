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
#include <nanvix/sys/perf.h>
#include <nanvix/config.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

/**
 * @brief Length of Inodes Table
 */
#define INODES_LENGTH (NANVIX_NR_INODES/4)

/**
 * @brief In-Memory Inode
 */
struct inode
{
	/* Must come first. */
	struct resource resource;

	struct d_inode data; /**< Underlying Disk Inode  */
	dev_t dev;           /**< Underlying Device      */
	ino_t num;           /**< Inode Number           */
	int count;           /**< Reference count        */
};

/**
 * @brief Table of Inodes
 */
static struct inode inodes[INODES_LENGTH];

/**
 * @brief Pool of Inodes
 */
static struct resource_pool pool = {
	.resources = inodes,
	.nresources = INODES_LENGTH,
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
	int idx;          /* inode index  */
	ino_t num;        /* Inode Number */
	struct inode *ip; /* Inode        */

	/* Invalid file system */
	if (fs == NULL)
		return (NULL);

	/* Allocate disk inode. */
	if ((num = minix_inode_alloc(fs->dev, &fs->super->data, fs->super->imap, mode, uid, gid)) == MINIX_INODE_NULL)
	{
		curr_proc->errcode = -EAGAIN;
		goto error0;
	}

	/* Allocate memory inode. */
	if ((idx = resource_alloc(&pool)) < 0)
	{
		curr_proc->errcode = -ENOMEM;
		goto error1;
	}

	ip = &inodes[idx];

	/* Read disk inode. */
	if (minix_inode_read(fs->dev, &fs->super->data, &ip->data, num) < 0)
		goto error2;

	/* Initialize inode. */
	ip->count = 1;
	ip->num = num;
	ip->dev = fs->dev;

	return (ip);

error2:
	resource_free(&pool, idx);
error1:
	uassert(minix_inode_free(&fs->super->data, fs->super->imap, num) == 0);
error0:
	return (NULL);
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
	if (!WITHIN(idx, 0, INODES_LENGTH))
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
		return(curr_proc->errcode = -EINVAL);

	/* Invalid inode. */
	if (ip == NULL)
		return(curr_proc->errcode = -EINVAL);

	idx = ip - inodes;

	/* Bad inode. */
	if (!WITHIN(idx, 0, INODES_LENGTH))
		return(curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (fs->dev != ip->dev)
		return(curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (ip->count == 0)
		return(curr_proc->errcode = -EINVAL);

	/* Write inode back to disk. */
	if (minix_inode_write(ip->dev, &fs->super->data, &ip->data, ip->num) < 0)
	{
		uprintf("[nanvix][vfs] failed to write inode %d", ip->num);
		return(curr_proc->errcode = -EAGAIN);
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
		return(curr_proc->errcode = -EINVAL);

	/* Invalid inode. */
	if (ip == NULL)
		return(curr_proc->errcode = -EINVAL);

	/* Bad inode. */
	if (fs->dev != ip->dev)
		return(curr_proc->errcode = -EINVAL);

	/* Write disk inode. */
	if (minix_inode_write(ip->dev, &fs->super->data, &ip->data, ip->num) < 0)
	{
		uprintf("[nanvix][vfs] failed to write inode %d", ip->num);
		return(curr_proc->errcode = -EAGAIN);
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
	/* Invalid file system */
	if (fs == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (NULL);
	}

	/* Search for inode in the table of inodes. */
	for (int i = 0; i < INODES_LENGTH; i++)
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
 * The inode_name() function lookups an inode by the name pointed to by
 * @p name. The root directory of @p fs is used as start point for the
 * search.
 *
 * @todo TODO: recursive lookup.
 * @todo TODO: support relative path search.
 */
struct inode *inode_name(struct filesystem *fs, const char *name)
{
	struct inode *dinode;   /* Directory's inode.     */
	off_t off;              /* Offset of Target Inode */
	struct d_dirent dirent; /* Directory Entry        */

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

	dinode = curr_proc->root;

	/* Failed to get directory. */
	if (dinode == NULL)
	{
		curr_proc->errcode = -EINVAL;
		return (NULL);
	}

	/* Search file. */
	if ((off = minix_dirent_search(fs->dev, &fs->super->data, fs->super->bmap, inode_disk_get(dinode), name, 0)) < 0)
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
	for (int i = 0; i < INODES_LENGTH; i++)
	{
		inodes[i].resource = RESOURCE_INITIALIZER;
		inodes[i].dev = -1;
		inodes[i].num = MINIX_INODE_NULL;
		inodes[i].count = 0;
	}
}
