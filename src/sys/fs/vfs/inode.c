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
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

/*============================================================================*
 * inode_alloc()                                                              *
 *============================================================================*/

/**
 * @brief Releases an in-memory inode.
 */
struct inode *inode_alloc(mode_t mode, uid_t uid, gid_t gid)
{
	ino_t num;        /* Inode Number */
	struct inode *ip; /* Inode        */

	/* Allocate disk inode. */
	if ((num = minix_inode_alloc(minix_fs.dev, &minix_fs.super, minix_fs.imap, mode, uid, gid)) == MINIX_INODE_NULL)
	{
		curr_proc->errcode = -EAGAIN;
		goto error0;
	}

	/* Allocate memory inode. */
	if ((ip = umalloc(sizeof(struct inode))) == NULL)
	{
		curr_proc->errcode = -ENOMEM;
		goto error1;
	}

	/* Read disk inode. */
	if (minix_inode_read(minix_fs.dev, &minix_fs.super, &ip->data, num) < 0)
		goto error2;

	/* Initialize inode. */
	ip->count = 1;
	ip->num = num;
	ip->dev = minix_fs.dev;

	return (ip);

error2:
	ufree(ip);
error1:
	uassert(minix_inode_free(&minix_fs.super, minix_fs.imap, num) == 0);
error0:
	return (NULL);
}

/*============================================================================*
 * inode_free()                                                               *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int inode_free(struct inode *ip)
{
	/* Invalid inode. */
	if (ip == NULL)
		return (-EINVAL);

	/* Bad inode. */
	if (ip->count == 0)
		return (-EINVAL);

	/*
	 * This inode is used by other processes. Let us just
	 * decrement the reference counter and return.
	 */
	if (ip->count-- > 1)
		return (0);

	/* Write inode back to disk. */
	if (minix_inode_write(ip->dev, &minix_fs.super, &ip->data, ip->num) < 0)
	{
		uprintf("[nanvix][vfs] failed to write inode %d", ip->num);
		return (-EAGAIN);
	}

	/* Release inode. */
	if (minix_inode_free(&minix_fs.super, minix_fs.imap, ip->num) < 0)
	{
		uprintf("[nanvix][vfs] failed to release inode %d", ip->num);
		return (-EAGAIN);
	}

	/* House keeping. */
	ufree(ip);

	return (0);
}

/*============================================================================*
 * inode_read()                                                               *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
struct inode *inode_read(ino_t num)
{
	struct inode *ip;

	/* Allocate memory inode. */
	if ((ip = umalloc(sizeof(struct inode))) == NULL)
	{
		curr_proc->errcode = -ENOMEM;
		goto error0;
	}

	/* Read disk inode. */
	if (minix_inode_read(minix_fs.dev, &minix_fs.super, &ip->data, num) < 0)
		goto error1;

	/* Initialize inode. */
	ip->count = 1;
	ip->num = num;
	ip->dev = minix_fs.dev;

	return (ip);

error1:
	ufree(ip);
error0:
	uassert(minix_inode_free(&minix_fs.super, minix_fs.imap, num) == 0);
	return (NULL);
}

/*============================================================================*
 * inode_read()                                                               *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int inode_write(struct inode *ip)
{
	/* Invalid inode. */
	if (ip == NULL)
		return (-EINVAL);

	/* Write disk inode. */
	if (minix_inode_write(ip->dev, &minix_fs.super, &ip->data, ip->num) < 0)
	{
		uprintf("[nanvix][vfs] failed to write inode %d", ip->num);
		return (-EAGAIN);
	}

	return (0);
}
