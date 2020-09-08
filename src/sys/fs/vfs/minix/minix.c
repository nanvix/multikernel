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
#define __NEED_LIMITS_FS

#include <nanvix/servers/vfs.h>
#include <nanvix/limits/fs.h>
#include <nanvix/dev.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

#define ROUND(x) (((x) == 0) ? 1 : (x))

/*============================================================================*
 * minix_dirent_search()                                                      *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
off_t minix_dirent_search(
	dev_t dev,
	struct d_superblock *super,
	bitmap_t *zmap,
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

	/* Invalid superblock. */
	if (super == NULL)
		return (-EINVAL);

	/* Invalid zone map. */
	if (zmap == NULL)
		return (-EINVAL);

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
				super,
				zmap,
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
				super,
				zmap,
				dip,
				i*sizeof(struct d_dirent),
				0
			);
			continue;
		}

		uassert(bdev_read(dev, (char *)&d, sizeof(struct d_dirent), base + off) == sizeof(struct d_dirent));

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
		blk = minix_block_map(super, zmap, dip, entry*sizeof(struct d_dirent), 1);
		dip->i_size += sizeof(struct d_dirent);
	}

	else
		blk = minix_block_map(super, zmap, dip, entry*sizeof(struct d_dirent), 0);

	/* Compute file offset. */
	off = (entry%(MINIX_BLOCK_SIZE/sizeof(struct d_dirent)))*sizeof(struct d_dirent);
	base = blk*MINIX_BLOCK_SIZE;

	return (base + off);
}

/*============================================================================*
 * minix_dirent_add()                                                         *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int minix_dirent_add(
	dev_t dev,
	struct d_superblock *super,
	bitmap_t *zmap,
	struct d_inode *dip,
	const char *name,
	minix_ino_t num
)
{
	off_t off;         /* File of the entry. */
	struct d_dirent d; /* Directory entry.   */

	/* Invalid superblock. */
	if (super == NULL)
		return (-EINVAL);

	/* Invalid zone map. */
	if (zmap == NULL)
		return (-EINVAL);

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
	if ((off = minix_dirent_search(dev, super, zmap, dip, name, 1)) < 0)
		return (-EAGAIN);

	/* Read directory entry. */
	uassert(bdev_read(dev, (char *)&d, sizeof(struct d_dirent), off) == sizeof(struct d_dirent));

	/* Set attributes. */
	d.d_ino = num;
	ustrncpy(d.d_name, name, MINIX_NAME_MAX);

	/* Write directory entry. */
	uassert(bdev_write(dev, (char *)&d, sizeof(struct d_dirent), off) == sizeof(struct d_dirent));

	dip->i_nlinks++;
	dip->i_time = 0;

	return (0);
}

/*============================================================================*
 * minix_dirent_remove()                                                      *
 *============================================================================*/

/**
 *
 * The minix_dirent_remove() function removes the directory entry named
 * @p name from the directory pointed to by @p dip.
 */
int minix_dirent_remove(
	dev_t dev,
	struct d_superblock *super,
	bitmap_t *zmap,
	struct d_inode *dip,
	const char *name
)
{
	struct d_inode ip; /* Target Inode        */
	off_t off;         /* Offset of the Entry */
	struct d_dirent d; /* Directory Entry     */

	/* Invalid superblock. */
	if (super == NULL)
		return (-EINVAL);

	/* Invalid zone map. */
	if (zmap == NULL)
		return (-EINVAL);

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
	if ((off = minix_dirent_search(dev, super, zmap, dip, name, 0)) < 0)
		return (-ENOENT);

	/* Read directory entry. */
	uassert(bdev_read(dev, (char *)&d, sizeof(struct d_dirent), off) == sizeof(struct d_dirent));

	/* Read inode. */
	if (minix_inode_read(dev, super, &ip, d.d_ino) < 0)
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
	if (minix_inode_write(dev, super, &ip, d.d_ino) < 0)
		return (-EAGAIN);

	/* Remove directory entry. */
	d.d_ino = MINIX_INODE_NULL;
	ustrncpy(d.d_name, "", MINIX_NAME_MAX);

	/* Write directory entry. */
	uassert(bdev_write(dev, (char *)&d, sizeof(struct d_dirent), off) == sizeof(struct d_dirent));

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
	size_t size;                    /* Size of file system.            */
	char buf[MINIX_BLOCK_SIZE];     /* Writing buffer.                 */
	minix_block_t imap_nblocks;     /* Number of inodes map blocks.    */
	minix_block_t bmap_nblocks;     /* Number of block map blocks.     */
	minix_block_t inode_nblocks;    /* Number of inode blocks.         */
	minix_block_t first_data_block; /* Number of inode blocks.         */
	mode_t mode;                    /* Access permissions to root dir. */
	minix_ino_t num;                /* Inode number of root directory. */
	struct d_inode winode;          /* Working Inode                   */
	struct d_superblock super;
	bitmap_t *imap;
	bitmap_t *bmap;
	struct d_inode root;

	/*
	 * TODO: sanity check arguments.
	 */

	/* Compute dimensions of file sytem. */
	imap_nblocks = ROUND(ninodes/(MINIX_BLOCK_BIT_LENGTH));
	bmap_nblocks = ROUND(nblocks/(MINIX_BLOCK_BIT_LENGTH));
	inode_nblocks = ROUND((ninodes*sizeof(struct d_inode))/MINIX_BLOCK_SIZE);

	/* Compute size of file system. */
	first_data_block  = 0;
	first_data_block += 1;             /* boot block   */
	first_data_block += 1;             /* superblock   */
	first_data_block += imap_nblocks;  /* inode map    */
	first_data_block += bmap_nblocks;  /* block map    */
	first_data_block += inode_nblocks; /* inode blocks */
	size = nblocks;                    /* data blocks  */
	size <<= MINIX_BLOCK_SIZE_LOG2;

	/* Fill file system with zeros. */
	umemset(buf, 0, MINIX_BLOCK_SIZE);
	for (size_t off = 0; off < size; off += MINIX_BLOCK_SIZE)
		uassert(bdev_write(dev, buf, MINIX_BLOCK_SIZE, off) == MINIX_BLOCK_SIZE);

	/* Write superblock. */
	super.s_ninodes = ninodes;
	super.s_nblocks = nblocks;
	super.s_imap_nblocks = imap_nblocks;
	super.s_bmap_nblocks = bmap_nblocks;
	super.s_first_data_block = first_data_block;
	super.s_max_size = NANVIX_MAX_FILE_SIZE;
	super.s_magic = MINIX_SUPER_MAGIC;

	/* Create inode and zone maps. */
	uassert((imap = ucalloc(imap_nblocks, MINIX_BLOCK_SIZE)) != NULL);
	uassert((bmap = ucalloc(bmap_nblocks, MINIX_BLOCK_SIZE)) != NULL);

	/* Access permission to root directory. */
	mode = S_IFDIR| S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

	uassert(minix_inode_alloc(dev, &super, imap, mode, uid, gid) == MINIX_INODE_ROOT);

	/* Create root directory. */
	uassert(minix_inode_read(dev, &super, &root, MINIX_INODE_ROOT) == 0);
	minix_dirent_add(dev, &super, bmap, &root, ".", MINIX_INODE_ROOT);
	minix_dirent_add(dev, &super,  bmap, &root, "..", MINIX_INODE_ROOT);
	uassert(minix_inode_write(dev, &super, &root, MINIX_INODE_ROOT) == 0);
	uprintf("[nanvix][vfs][minix] root inode = %d", MINIX_INODE_ROOT);

	/* Create disk device. */
	uassert((num = minix_inode_alloc(dev, &super, imap, mode | S_IFBLK, uid, gid)) != MINIX_INODE_NULL);
	uassert(minix_inode_read(dev, &super, &winode, num) == 0);
	winode.i_size = NANVIX_DISK_SIZE;
	minix_dirent_add(dev, &super, bmap, &root, "disk", num);
	uassert(minix_inode_write(dev, &super, &winode, MINIX_INODE_ROOT) == 0);
	uassert(minix_inode_write(dev, &super, &root, MINIX_INODE_ROOT) == 0);
	uprintf("[nanvix][vfs][minix] disk inode = %d", num);

	uprintf("[nanvix][vfs][minix] first data block =  %d", super.s_first_data_block);

	/* Write superblock */
	uassert(minix_super_write(dev, &super, bmap, imap) == 0);

	/* House keeping. */
	ufree(bmap);
	ufree(imap);

	return (0);
}

/*============================================================================*
 * minix_sync()                                                               *
 *============================================================================*/

/**
 * The minix_sync() function synchronizes the MINIX file system that
 * resides in the device specified by @p dev with in-memory file system
 * information. Any changes made to the file system structure are
 * flushed back to disk.
 */
int minix_sync(
	struct d_superblock *super,
	bitmap_t *imap,
	bitmap_t *bmap,
	dev_t dev
)
{
	/* Sanity check */
	if ((super == NULL) || (imap == NULL) || (bmap == NULL))
		return (-EINVAL);

	/* Write superblock. */
	if (minix_super_write(dev, super, bmap, imap) < 0)
		return (-EIO);

	return (0);
}

/*============================================================================*
 * minix_mount()                                                              *
 *============================================================================*/

/**
 * The minix_mount() function mounts the MINIX file system that resides
 * in the device specified by @p dev. The information concerning the
 * file system, such as superblock, inode map, block map, root inode are
 * placed in @p super, @p imap and @p bmap, respectively.
 */
int minix_mount(
	struct d_superblock *super,
	bitmap_t **imap,
	bitmap_t **bmap,
	dev_t dev
)
{
	/* Sanity check */
	if ((super == NULL) || (imap == NULL) || (bmap == NULL))
		return (-EINVAL);

	/* Read superblock. */
	if (minix_super_read(dev, super, bmap, imap) < 0)
		return (-EIO);

	return (0);
}

/*============================================================================*
 * minix_unmount()                                                            *
 *============================================================================*/

/**
 * The minix_unmount() function unmounts the MINIX file system that
 * resides in the device specified by @p dev. Any changes made to the
 * file system structure are flushed back to disk.
 */
int minix_unmount(
	struct d_superblock *super,
	bitmap_t *imap,
	bitmap_t *bmap,
	dev_t dev
)
{
	return (minix_sync(super, imap, bmap, dev));
}
