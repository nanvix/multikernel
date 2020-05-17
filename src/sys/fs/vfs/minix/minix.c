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

#include <posix/sys/types.h>
#include <posix/sys/stat.h>
#include <posix/stdint.h>
#include <nanvix/dev.h>
#include <nanvix/ulib.h>
#include "../include/minix.h"

#define ROUND(x) (((x) == 0) ? 1 : (x))

/**
 * @brief Minix File System Information
 */
struct minix_fs_info minix_fs;

/**
 * @brief Searches for a directory entry.
 *
 * @param ip       Directory where the directory entry shall be searched.
 * @param filename Name of the directory entry that shall be searched.
 * @param create   Create directory entry?
 *
 * @returns The file offset where the directory entry is located, or -1 if the
 *          file does not exist.
 *
 * @note @p ip must point to a valid inode
 * @note @p filename must point to a valid file name.
 * @note The Minix file system must be mounted.
 */
static off_t dirent_search(struct d_inode *ip, const char *filename,bool create)
{
	int i;             /* Working entry.               */
	off_t base, off;   /* Working file offsets.        */
	int entry;         /* Free entry.                  */
	minix_block_t blk;       /* Working block.               */
	int nentries;      /* Number of directory entries. */
	struct d_dirent d; /* Working directory entry.     */

	nentries = ip->i_size/sizeof(struct d_dirent);

	/* Search for directory entry. */
	i = 0;
	entry = -1;
	blk = ip->i_zones[0];
	base = -1;
	while (i < nentries)
	{
		/* Skip invalid block. */
		if (blk == MINIX_BLOCK_NULL)
		{
			i += MINIX_BLOCK_SIZE/sizeof(struct d_dirent);
			blk = minix_block_map(&minix_fs.super, minix_fs.zmap, ip, i*sizeof(struct d_dirent), false);
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
			blk = minix_block_map(&minix_fs.super, minix_fs.zmap, ip, i*sizeof(struct d_dirent), false);
			continue;
		}

		bdev_read(0, (char *)&d, sizeof(struct d_dirent), base);

		/* Valid entry. */
		if (d.d_ino != MINIX_INODE_NULL)
		{
			/* Found. */
			if (!ustrncmp(d.d_name, filename, MINIX_NAME_MAX))
			{
				/* Duplicate entry. */
				if (create)
					upanic("duplicate entry");

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
		blk = minix_block_map(&minix_fs.super, minix_fs.zmap, ip, entry*sizeof(struct d_dirent), true);
		ip->i_size += sizeof(struct d_dirent);
	}

	else
		blk = minix_block_map(&minix_fs.super, minix_fs.zmap, ip, entry*sizeof(struct d_dirent), false);

	/* Compute file offset. */
	off = (entry%(MINIX_BLOCK_SIZE/sizeof(struct d_dirent)))*sizeof(struct d_dirent);
	base = blk*MINIX_BLOCK_SIZE;

	return (base + off);
}

/**
 * @brief Searches for a file in a directory.
 *
 * @param dip       Directory where the file shall be searched.
 * @param filename File that shal be searched.
 *
 * @returns The inode number of the requested file, or #MINIX_INODE_NULL, if such
 *          files does not exist.
 *
 * @note @p dip must point to a valid inode.
 * @note @p filename must point to a valid file name.
 * @note The Minix file system must be mounted.
 */
minix_ino_t dir_search(struct d_inode *dip, const char *filename)
{
	off_t off;         /* File offset where the entry is. */
	struct d_dirent d; /* Working directory entry.        */

	/* Not a directory. */
	if (!S_ISDIR(dip->i_mode))
		upanic("not a directory");

	/* Search directory entry. */
	off = dirent_search(dip, filename, false);
	if (off == -1)
		return (MINIX_INODE_NULL);

	bdev_read(0, (char *)&d, sizeof(struct d_dirent), off);

	return (d.d_ino);
}

/**
 * @brief Adds an entry in a directory.
 *
 * @param dip       Directory where the entry should be added.
 * @param filename Name of the entry.
 * @param num      Inode number of the entry.
 *
 * @note @p dip must point to a valid inode.
 * @note @p filename must point to a valid file name.
 * @note The Minix file system must be mounted.
 */
static void minix_dirent_add
(struct d_inode *dip, const char *filename, minix_ino_t num)
{
	off_t off;         /* File offset of the entry. */
	struct d_dirent d; /* Directory entry.          */

	/* Get free entry. */
	off = dirent_search(dip, filename, true);

	/* Read directory entry. */
	bdev_read(0, (char *)&d, sizeof(struct d_dirent), off);

	/* Set attributes. */
	d.d_ino = num;
	ustrncpy(d.d_name, filename, MINIX_NAME_MAX);

	/* Write directory entry. */
	bdev_write(0, (char *)&d, sizeof(struct d_dirent), off);

	dip->i_nlinks++;
	dip->i_time = 0;
}

/**
 * @brief Creates a Minix file system.
 *
 * @param ninodes  Number of inodes.
 * @param nblocks  Number of blocks.
 * @param uid  User ID.
 * @param gid  User group ID.
 *
 * @note @p diskfile must refer to a valid file.
 * @note @p ninodes must be valid.
 * @note @p nblocks must be valid.
 */
int minix_mkfs(
	minix_ino_t ninodes,
	minix_block_t nblocks,
	uint16_t uid,
	uint16_t gid
)
{
	size_t size;            /* Size of file system.            */
	char buf[MINIX_BLOCK_SIZE];   /* Writing buffer.                 */
	minix_block_t imap_nblocks;  /* Number of inodes map blocks.    */
	minix_block_t bmap_nblocks;  /* Number of block map blocks.     */
	minix_block_t inode_nblocks; /* Number of inode blocks.         */
	struct d_inode root;   /* Root directory.                 */
	mode_t mode;            /* Access permissions to root dir. */
	minix_ino_t num;           /* Inode number of root directory. */

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
		bdev_write(0, buf, MINIX_BLOCK_SIZE, i*MINIX_BLOCK_SIZE);

	/* Write superblock. */
	minix_fs.super.s_ninodes = ninodes;
	minix_fs.super.s_nblocks = nblocks;
	minix_fs.super.s_imap_nblocks = imap_nblocks;
	minix_fs.super.s_bmap_nblocks = bmap_nblocks;
	minix_fs.super.s_first_data_block = 2 + imap_nblocks + bmap_nblocks + inode_nblocks;
	minix_fs.super.s_max_size = 532480;
	minix_fs.super.s_magic = MINIX_SUPER_MAGIC;

	/* Create inode map. */
	uassert((minix_fs.imap = ucalloc(imap_nblocks, MINIX_BLOCK_SIZE)) != NULL);

	/* Create block map. */
	uassert((minix_fs.zmap = ucalloc(bmap_nblocks, MINIX_BLOCK_SIZE)) != NULL);

	/* Access permission to root directory. */
	mode  = S_IFDIR| S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

	/* Create root directory. */
	num = minix_inode_alloc(&minix_fs.super, minix_fs.imap, mode, uid, gid);
	minix_inode_read(&minix_fs.super, &root, num);
	minix_dirent_add(&root, ".", num);
	minix_dirent_add(&root, "..", num);
	root.i_nlinks--;
	minix_inode_write(&minix_fs.super, &root, num);

	return (minix_super_write(NANVIX_ROOT_DEV, &minix_fs.super, minix_fs.zmap, minix_fs.imap));
}
