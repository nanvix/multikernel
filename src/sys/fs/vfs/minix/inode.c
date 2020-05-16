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
 * @brief Reads an inode from the currently mounted Minix file system.
 *
 * @param num Number of the inode that shall be read.
 *
 * @returns A pointer to the requested inode.
 *
 * @note The Minix file system must be mounted.
 */
struct d_inode *minix_inode_read(struct d_superblock *sb, struct d_inode *ip, minix_ino_t num)
{
	unsigned idx, off;         /* Inode number offset/index. */
	off_t offset;              /* Offset in the file system. */
	unsigned inodes_per_block; /* Inodes per block.          */

	num--;

	/* Bad inode number. */
	if (num >= sb->s_imap_nblocks*MINIX_BLOCK_BIT_LENGTH)
		upanic("bad inode number");

	/* Compute file offset. */
	inodes_per_block = MINIX_BLOCK_SIZE/sizeof(struct d_inode);
	idx = num/inodes_per_block;
	off = num%inodes_per_block;
	offset = (2 + sb->s_imap_nblocks + sb->s_bmap_nblocks + idx)*MINIX_BLOCK_SIZE;
	offset += off*sizeof(struct d_inode);

	/* Read inode. */
	bdev_read(0, (char *)ip, sizeof(struct d_inode), offset);

	return (ip);
}

/**
 * @brief Writes an inode to the currently mounted Minix file system.
 *
 * @param num Number of the inode that shall be written.
 * @param ip  Inode that shall be written.
 *
 * @note The Minix file system must be mounted.
 */
void minix_inode_write(struct d_superblock *sb, struct d_inode *ip, minix_ino_t num)
{
	unsigned idx, off;         /* Inode number offset/index. */
	off_t offset;              /* Offset in the file system. */
	unsigned inodes_per_block; /* Inodes per block.          */

	num--;

	/* Bad inode number. */
	if (num >= sb->s_imap_nblocks*MINIX_BLOCK_BIT_LENGTH)
		upanic("bad inode number");

	/* Compute file offset. */
	inodes_per_block = MINIX_BLOCK_SIZE/sizeof(struct d_inode);
	idx = num/inodes_per_block;
	off = num%inodes_per_block;
	offset = (2 + sb->s_imap_nblocks + sb->s_bmap_nblocks + idx)*MINIX_BLOCK_SIZE;
	offset += off*sizeof(struct d_inode);

	/* Write inode. */
	bdev_write(0, (char *)ip, sizeof(struct d_inode), offset);
}

/**
 * @brief Allocates an inode.
 *
 * @param mode Access mode.
 * @param uid  User ID.
 * @param gid  User group ID.
 *
 * @returns The number of the allocated inode.
 *
 * @note The Minix file system must be mounted.
 */
minix_ino_t minix_inode_alloc(struct d_superblock *sb, bitmap_t *imap, uint16_t mode, uint16_t uid, uint16_t gid)
{
	minix_ino_t num;      /* Inode number.                */
	bitmap_t bit;      /* Bit number if the inode map. */
	struct d_inode ip; /* New inode.                   */

	/* Allocate inode. */
	bit = bitmap_first_free(imap, sb->s_imap_nblocks*MINIX_BLOCK_BIT_LENGTH);
	if (bit == BITMAP_FULL)
		upanic("inode map overflow");
	bitmap_set(imap, bit);
	num = bit + 1;

	/* Initialize inode. */
	minix_inode_read(sb, &ip, num);
	ip.i_mode = mode;
	ip.i_uid = uid;
	ip.i_size = 0;
	ip.i_time = 0;
	ip.i_gid = gid;
	ip.i_nlinks = 1;
	for (unsigned i = 0; i < MINIX_NR_ZONES; i++)
		ip.i_zones[i] = MINIX_BLOCK_NULL;
	minix_inode_write(sb, &ip, num);

	return (num);
}
