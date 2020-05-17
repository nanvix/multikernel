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

/**
 * @brief Reads the superblock of a Minix file system.
 */
void minix_super_read(struct d_superblock *sb, bitmap_t *imap, bitmap_t *zmap)
{
	/* Read superblock. */
	bdev_read(0, (char  *)sb, sizeof(struct d_superblock), 1*MINIX_BLOCK_SIZE);
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		upanic("bad magic number");

	/* Read inode map. */
	bdev_read(0, (char *)imap, sb->s_imap_nblocks*MINIX_BLOCK_SIZE, 2*MINIX_BLOCK_SIZE);

	/* Read block map. */
	bdev_read(0, (char *)zmap, sb->s_bmap_nblocks*MINIX_BLOCK_SIZE, (2 + sb->s_imap_nblocks)*MINIX_BLOCK_SIZE);
}

/**
 * @brief Writes the superblock of a Minix file system.
 */
void minix_super_write(struct d_superblock *sb, bitmap_t *imap, bitmap_t *zmap)
{
	/* Write superblock. */
	bdev_write(0, (char *)sb, sizeof(struct d_superblock), 1*MINIX_BLOCK_SIZE);

	/* Write inode map. */
	bdev_write(0, (char *)imap, sb->s_imap_nblocks*MINIX_BLOCK_SIZE, 2*MINIX_BLOCK_SIZE);

	/* Write zone map. */
	bdev_write(0, (char *)zmap, sb->s_bmap_nblocks*MINIX_BLOCK_SIZE, (2 + sb->s_imap_nblocks)*MINIX_BLOCK_SIZE);
}
