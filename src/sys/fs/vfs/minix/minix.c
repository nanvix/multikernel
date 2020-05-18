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
#define __NEED_LIMITS_FS

#include <nanvix/limits/fs.h>
#include <posix/sys/types.h>
#include <posix/sys/stat.h>
#include <posix/errno.h>
#include <nanvix/dev.h>
#include <nanvix/ulib.h>
#include "../include/minix.h"

#define ROUND(x) (((x) == 0) ? 1 : (x))

/**
 * @brief Minix File System Information
 */
struct minix_fs_info minix_fs;

/*============================================================================*
 * minix_dirent_search()                                                      *
 *============================================================================*/

/**
 * @brief Searches for a directory entry.
 *
 * The minix_dirent_search() function searches in the target directory
 * pointed to by @p dip for the entry named @p name. If the entry does
 * not exist and @p create is non zero, the entry is created.
 *
 * @param dip    Directory where the directory entry shall be searched.
 * @param name   Name of the directory entry that shall be searched.
 * @param create Create directory entry?
 *
 * @returns The file offset where the directory entry is located, or -1
 * if the file does not exist.
 */
off_t minix_dirent_search(
	struct d_inode *dip,
	const char *name,
	int create
)
{
	int i;             /* Working entry.               */
	off_t base, off;   /* Working file offsets.        */
	int entry;         /* Free entry.                  */
	minix_block_t blk; /* Working block.               */
	int nentries;      /* Number of directory entries. */
	struct d_dirent d; /* Working directory entry.     */

	/* Invalid directory pointer. */
	if (dip == NULL)
		return (-EINVAL);

	/* Not a directory. */
	if (!S_ISDIR(dip->i_mode))
		return (-EINVAL);

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Bad name. */
	if (ustrlen(name) > MINIX_NAME_MAX)
		return (-ENAMETOOLONG);

	nentries = dip->i_size/sizeof(struct d_dirent);

	/* Search for directory entry. */
	entry = -1;
	blk = dip->i_zones[0];
	off = 0; base = -1;
	for (i = 0; i < nentries; /* noop*/)
	{
		/* Skip invalid block. */
		if (blk == MINIX_BLOCK_NULL)
		{
			i += MINIX_BLOCK_SIZE/sizeof(struct d_dirent);
			blk = minix_block_map(
				&minix_fs.super,
				minix_fs.zmap,
				dip,
				i*sizeof(struct d_dirent),
				0
			);
			continue;
		}

		/* Compute file offset. */
		if (base < 0)
		{
			off = 0;
			base = blk*MINIX_BLOCK_SIZE;
		}

		/* Get next block. */
		else if (off >= MINIX_BLOCK_SIZE)
		{
			base = -1;
			blk = minix_block_map(
				&minix_fs.super,
				minix_fs.zmap,
				dip,
				i*sizeof(struct d_dirent),
				0
			);
			continue;
		}

		bdev_read(minix_fs.dev, (char *)&d, sizeof(struct d_dirent), base + off);

		/* Valid entry. */
		if (d.d_ino != MINIX_INODE_NULL)
		{
			/* Found. */
			if (!ustrncmp(d.d_name, name, MINIX_NAME_MAX))
			{
				/* Duplicate entry. */
				if (create)
					return (-EEXIST);

				return (base + off);
			}
		}

		/* Remember entry index. */
		else
			entry = i;

		i++;
		off += sizeof(struct d_dirent);
	}

	/* No entry found. */
	if (!create)
		return (-1);

	/* Expand directory. */
	if (entry < 0)
	{
		entry = nentries;
		blk = minix_block_map(&minix_fs.super, minix_fs.zmap, dip, entry*sizeof(struct d_dirent), 1);
		dip->i_size += sizeof(struct d_dirent);
	}

	else
		blk = minix_block_map(&minix_fs.super, minix_fs.zmap, dip, entry*sizeof(struct d_dirent), 0);

	/* Compute file offset. */
	off = (entry%(MINIX_BLOCK_SIZE/sizeof(struct d_dirent)))*sizeof(struct d_dirent);
	base = blk*MINIX_BLOCK_SIZE;

	return (base + off);
}

/*============================================================================*
 * minix_dirent_add()                                                         *
 *============================================================================*/

/**
 * @brief Adds an entry in a directory.
 *
 * The minix_dirent_add() function adds a new entry to the directory
 * pointed to by @p dip.  The new entry is set to point to the inode @p
 * num and is linked to the symbolic name pointed to by @p name.
 *
 * @param dip  Target directory.
 * @param name Name of the entry.
 * @param num  Number of linked inode.
 *
 * @returns upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int minix_dirent_add(
	struct d_inode *dip,
	const char *name,
	minix_ino_t num
)
{
	off_t off;         /* File of the entry. */
	struct d_dirent d; /* Directory entry.   */

	/* Invalid directory pointer. */
	if (dip == NULL)
		return (-EINVAL);

	/* Not a directory. */
	if (!S_ISDIR(dip->i_mode))
		return (-EINVAL);

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Bad name. */
	if (ustrlen(name) > MINIX_NAME_MAX)
		return (-ENAMETOOLONG);

	/* Invalid inode. */
	if (num == MINIX_INODE_NULL)
		return (-EINVAL);

	/* Get free entry. */
	if ((off = minix_dirent_search(dip, name, 1)) < 0)
		return (-EAGAIN);

	/* Read directory entry. */
	bdev_read(minix_fs.dev, (char *)&d, sizeof(struct d_dirent), off);

	/* Set attributes. */
	d.d_ino = num;
	ustrncpy(d.d_name, name, MINIX_NAME_MAX);

	/* Write directory entry. */
	bdev_write(minix_fs.dev, (char *)&d, sizeof(struct d_dirent), off);

	dip->i_nlinks++;
	dip->i_time = 0;

	return (0);
}

/*============================================================================*
 * minix_dirent_remove()                                                      *
 *============================================================================*/

/**
 * @brief Removes a directory entry.
 *
 * @param dip  Target directory.
 * @param name Symbolic name of target entry.
 *
 * The nanvix_dirent_remove() function removes the directory entry named
 * @p name from the directory pointed to by @p dip.
 *
 * @returns Upon successful return, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int minix_dirent_remove(
	struct d_inode *dip,
	const char *name
)
{
	struct d_inode ip; /* Target Inode        */
	off_t off;         /* Offset of the Entry */
	struct d_dirent d; /* Directory Entry     */
	/* Invalid directory. */
	if (dip == NULL)
		return (-EINVAL);

	/* Not a directory. */
	if (!S_ISDIR(dip->i_mode))
		return (-EINVAL);

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Bad name. */
	if (ustrlen(name) > MINIX_NAME_MAX)
		return (-ENAMETOOLONG);

	/* Cannot remove '.' */
	if (!ustrcmp(name, "."))
		return (-EBUSY);

	/* Search entry. */
	if ((off = minix_dirent_search(dip, name, 0)) < 0)
		return (-ENOENT);

	/* Read directory entry. */
	bdev_read(minix_fs.dev, (char *)&d, sizeof(struct d_dirent), off);

	/* Read inode. */
	if (minix_inode_read(minix_fs.dev, &minix_fs.super, &ip, d.d_ino) < 0)
		return (-ENOENT);

	/* Unlinking directory. */
	if (S_ISDIR(ip.i_mode))
	{
		/* Directory not empty. */
		if (ip.i_size > 0)
			return (-EBUSY);
	}

	/* Write inode. */
	ip.i_nlinks--;
	if (minix_inode_write(minix_fs.dev, &minix_fs.super, &ip, d.d_ino) < 0)
		return (-EAGAIN);

	/* Remove directory entry. */
	d.d_ino = MINIX_INODE_NULL;
	ustrncpy(d.d_name, "", MINIX_NAME_MAX);

	/* Write directory entry. */
	bdev_write(minix_fs.dev, (char *)&d, sizeof(struct d_dirent), off);

	return (0);
}

/*============================================================================*
 * minix_mkfs()                                                               *
 *============================================================================*/

/**
 * The minix_mkfs() function creates a MINIX file system in the device
 * @p dev. The file system is formatted to feature @p ninode inodes, @p
 * nblocks @p nblocks. Furthermore, the user ID and the user group ID of
 * the file system are set to @p uid, and @p gid, respectively.
 */
int minix_mkfs(
	dev_t dev,
	minix_ino_t ninodes,
	minix_block_t nblocks,
	minix_uid_t uid,
	minix_gid_t gid
)
{
	size_t size;                 /* Size of file system.            */
	char buf[MINIX_BLOCK_SIZE];  /* Writing buffer.                 */
	minix_block_t imap_nblocks;  /* Number of inodes map blocks.    */
	minix_block_t bmap_nblocks;  /* Number of block map blocks.     */
	minix_block_t inode_nblocks; /* Number of inode blocks.         */
	mode_t mode;                 /* Access permissions to root dir. */
	minix_ino_t num;             /* Inode number of root directory. */

	/*
	 * TODO: sanity check arguments.
	 */

	/* Compute dimensions of file sytem. */
	imap_nblocks = ROUND(ninodes/(MINIX_BLOCK_BIT_LENGTH));
	bmap_nblocks = ROUND(nblocks/(MINIX_BLOCK_BIT_LENGTH));
	inode_nblocks = ROUND( (ninodes*sizeof(struct d_inode))/MINIX_BLOCK_SIZE);

	/* Compute size of file system. */
	size  = 1;             /* boot block   */
	size += 1;             /* superblock   */
	size += imap_nblocks;  /* inode map    */
	size += bmap_nblocks;  /* block map    */
	size += inode_nblocks; /* inode blocks */
	size += nblocks;       /* data blocks  */
	size <<= MINIX_BLOCK_SIZE_LOG2;

	/* Fill file system with zeros. */
	umemset(buf, 0, MINIX_BLOCK_SIZE);
	for (size_t i = 0; i < size; i += MINIX_BLOCK_SIZE)
		bdev_write(minix_fs.dev, buf, MINIX_BLOCK_SIZE, i*MINIX_BLOCK_SIZE);

	minix_fs.dev = dev;

	/* Write superblock. */
	minix_fs.super.s_ninodes = ninodes;
	minix_fs.super.s_nblocks = nblocks;
	minix_fs.super.s_imap_nblocks = imap_nblocks;
	minix_fs.super.s_bmap_nblocks = bmap_nblocks;
	minix_fs.super.s_first_data_block = 2 + imap_nblocks + bmap_nblocks + inode_nblocks;
	minix_fs.super.s_max_size = NANVIX_MAX_FILE_SIZE;
	minix_fs.super.s_magic = MINIX_SUPER_MAGIC;

	/* Create inode and zone maps. */
	uassert((minix_fs.imap = ucalloc(imap_nblocks, MINIX_BLOCK_SIZE)) != NULL);
	uassert((minix_fs.zmap = ucalloc(bmap_nblocks, MINIX_BLOCK_SIZE)) != NULL);

	/* Access permission to root directory. */
	mode  = S_IFDIR| S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

	/* Create root directory. */
	uassert((num = minix_inode_alloc(dev, &minix_fs.super, minix_fs.imap, mode, uid, gid)) != MINIX_INODE_NULL);
	minix_fs.root_ino = num;
	minix_inode_read(dev, &minix_fs.super, &minix_fs.root, num);
	minix_dirent_add(&minix_fs.root, ".", num);
	minix_dirent_add(&minix_fs.root, "..", num);
	uassert(minix_inode_write(dev, &minix_fs.super, &minix_fs.root, num) == 0);

	return (minix_super_write(dev, &minix_fs.super, minix_fs.zmap, minix_fs.imap));
}
