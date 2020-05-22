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

/*============================================================================*
 * minix_super_read()                                                         *
 *============================================================================*/

/**
 * The minix_super_read() function reads the superblock device of the
 * MINIX file system stored in the device @p dev. The superblock that
 * was read is stored in the location pointed to by @p sb, as well as
 * the inode map is placed in the location pointed to by @p imap, and
 * the zone map is written to the location pointed to by @p zmap.
 */
int minix_super_read(
	dev_t dev,
	struct d_superblock *sb,
	bitmap_t *zmap,
	bitmap_t *imap
)
{
	int err;

	/* Superblock */
	err = bdev_read(
		dev,
		(char *)sb,
		sizeof(struct d_superblock),
		1*MINIX_BLOCK_SIZE
	);

	/* Read failure. */
	if (err < 0)
		return (err);

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (-EINVAL);

	/* I-Node Map */
	err = bdev_read(
		dev,
		(char *)imap,
		sb->s_imap_nblocks*MINIX_BLOCK_SIZE,
		2*MINIX_BLOCK_SIZE
	);

	/* Read failure. */
	if (err < 0)
		return (err);

	/* Zone Map */
	err = bdev_read(
		dev,
		(char *)zmap,
		sb->s_bmap_nblocks*MINIX_BLOCK_SIZE,
		(2 + sb->s_imap_nblocks)*MINIX_BLOCK_SIZE
	);

	/* Read failure. */
	if (err < 0)
		return (err);

	return (0);
}

/*============================================================================*
 * minix_super_write()                                                        *
 *============================================================================*/

/**
 * The minix_super_write() function writes the superblock device of the
 * MINIX file system stored in the device @p dev. The superblock to be
 * written is read from the location pointed to by @p sb, as well as the
 * inode map is retrieved from the location pointed to by @p imap, and
 * the zone map is get from the location pointed to by @p zmap.
 */
int minix_super_write(
	dev_t dev,
	const struct d_superblock *sb,
	const bitmap_t *zmap,
	const bitmap_t *imap
)
{
	int err;

	/* Superblock */
	err = bdev_write(
		dev,
		(char *)sb,
		sizeof(struct d_superblock),
		1*MINIX_BLOCK_SIZE
	);

	/* Write failure. */
	if (err < 0)
		return (err);

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (-EINVAL);

	/* I-Node Map */
	err = bdev_write(
		dev,
		(char *)imap,
		sb->s_imap_nblocks*MINIX_BLOCK_SIZE,
		2*MINIX_BLOCK_SIZE
	);

	/* Write failure. */
	if (err < 0)
		return (err);

	/* Zone Map */
	err = bdev_write(
		dev,
		(char *)zmap,
		sb->s_bmap_nblocks*MINIX_BLOCK_SIZE,
		(2 + sb->s_imap_nblocks)*MINIX_BLOCK_SIZE
	);

	/* Write failure. */
	if (err < 0)
		return (err);

	return (0);
}
