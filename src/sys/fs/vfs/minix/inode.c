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
#include <nanvix/dev.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

/**
 * @brief Inodes per Block
 */
#define MINIX_INODES_PER_BLOCK (MINIX_BLOCK_SIZE/sizeof(struct d_inode))

/*============================================================================*
 * minix_inode_offset()                                                       *
 *============================================================================*/

/**
 * @brief Computes the offset of an inode.
 *
 * @param sb  Target file system.
 * @param num Inode number.
 *
 * @returns The offset of the target inode @p num in the file system
 * pointed to by @p sb.
 */
static off_t minix_inode_offset(struct d_superblock *sb, minix_ino_t num)
{
	off_t idx, off; /* Inode number offset/index. */
	off_t offset;   /* Offset in the file system. */

	idx = num/MINIX_INODES_PER_BLOCK;
	off = num%MINIX_INODES_PER_BLOCK;
	offset = (2 + sb->s_imap_nblocks + sb->s_bmap_nblocks + idx)*MINIX_BLOCK_SIZE;
	offset += off*sizeof(struct d_inode);

	return (offset);
}

/*============================================================================*
 * minix_inode_read()                                                         *
 *============================================================================*/

/**
 * The minix_inode_read() function reads the inode @p num from the MINIX
 * file system pointed to by @p sb that resides in the device @p dev.
 * The inode is written to inode the location pointed to by @p ip.
 */
int minix_inode_read(
	dev_t dev,
	struct d_superblock *sb,
	struct d_inode *ip,
	minix_ino_t num
)
{
	off_t offset;

	/* Invalid superblock. */
	if (sb == NULL)
		return (-EINVAL);

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (-EINVAL);

	/* Invalid inode. */
	if (ip == NULL)
		return (-EINVAL);

	num--;

	/* Bad inode number. */
	if (num >= sb->s_ninodes)
		return (-EINVAL);

	/* Read inode. */
	offset = minix_inode_offset(sb, num);
	if (bdev_read(dev, (char *)ip, sizeof(struct d_inode), offset) < 0)
		return (-EAGAIN);

	return (0);
}

/*============================================================================*
 * minix_inode_write()                                                        *
 *============================================================================*/

/**
 * The minix_inode_write() function writes the inode pointed to by @p ip
 * in the MINIX file system pointed to by @p sb that resides in the
 * device @p dev. The inode is written to inode number @p num.
 */
int minix_inode_write(
	dev_t dev,
	struct d_superblock *sb,
	struct d_inode *ip,
	minix_ino_t num
)
{
	off_t offset;

	/* Invalid superblock. */
	if (sb == NULL)
		return (-EINVAL);

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (-EINVAL);

	/* Invalid inode. */
	if (ip == NULL)
		return (-EINVAL);

	num--;

	/* Bad inode number. */
	if (num >= sb->s_ninodes)
		return (-EINVAL);

	/* Write inode. */
	offset = minix_inode_offset(sb, num);
	if (bdev_write(dev, (char *)ip, sizeof(struct d_inode), offset) < 0)
		return (-EAGAIN);

	return (0);
}

/*============================================================================*
 * minix_inode_alloc()                                                        *
 *============================================================================*/

/**
 * The minix_inode_alloc() function allocates a new inode in the MINIX
 * file system pointed to by @p sb that resides in the device @p dev.
 * The inode is allocated in the inode map pointed to by @p imap, and it
 * is initialized with @p mode, @p uid and @p gid, access mode, user ID
 * and group ID, respectively.
 *
 * @todo TODO: write timestamp to inode.
 */
minix_ino_t minix_inode_alloc(
	dev_t dev,
	struct d_superblock *sb,
	bitmap_t *imap,
	minix_mode_t mode,
	minix_uid_t uid,
	minix_gid_t gid
)
{
	minix_ino_t num;   /* Inode number                */
	bitmap_t bit;      /* Bit Number in the Inode Map */
	struct d_inode ip; /* New inode                   */

	/* Invalid superblock. */
	if (sb == NULL)
		return (MINIX_INODE_NULL);

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (MINIX_INODE_NULL);

	/* Invalid inode map. */
	if (imap == NULL)
		return (MINIX_INODE_NULL);

	/* Allocate inode. */
	bit = bitmap_first_free(
		imap,
		sb->s_ninodes
	);
	if (bit == BITMAP_FULL)
	{
		uprintf("[nanvix][vfs] minix imap overflow");
		return (MINIX_INODE_NULL);
	}
	bitmap_set(imap, bit);
	num = bit + 1;

	/* Initialize inode. */
	minix_inode_read(dev, sb, &ip, num);
	ip.i_mode = mode;
	ip.i_uid = uid;
	ip.i_size = 0;
	ip.i_time = 0;
	ip.i_gid = gid;
	ip.i_nlinks = 1;
	for (unsigned i = 0; i < MINIX_NR_ZONES; i++)
		ip.i_zones[i] = MINIX_BLOCK_NULL;

	/* Write inode to disk. */
	if (minix_inode_write(dev, sb, &ip, num) < 0)
		goto error0;

	return (num);

error0:
	bitmap_clear(imap, bit);
	return (MINIX_INODE_NULL);
}

/*============================================================================*
 * minix_inode_free()                                                         *
 *============================================================================*/

/**
 * The minix_inode_free() function releases the inode @p num that is
 * allocated in the inode map pointed to by @p imap and resides in the
 * file system pointed to by @p sb.
 */
int minix_inode_free(struct d_superblock *sb, bitmap_t *imap, minix_ino_t num)
{
	/* Invalid superblock. */
	if (sb == NULL)
		return (-EINVAL);

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (-EINVAL);

	/* Invalid inode map. */
	if (imap == NULL)
		return (-EINVAL);

	num--;

	/* Bad inode number. */
	if (num >= sb->s_ninodes)
		return (-EINVAL);

	bitmap_clear(imap, num);

	return (0);
}
