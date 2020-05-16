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

#ifndef _MINIX_H_
#define _MINIX_H_

	#include <fs/minix.h>
	#include <nanvix/config.h>

	/**
	 * @brief Number of Bits in a Block
	 */
	#define MINIX_BLOCK_BIT_LENGTH (8*MINIX_BLOCK_SIZE)

	/**
	 * @brief File System Information
	 */
	struct minix_fs_info
	{

		/**
		 * @brief Mounted superblock.
		 */
		struct d_superblock super;

		/**
		 * @brief Inode map.
		 */
		bitmap_t *imap;

		/**
		 * @brief Zone map.
		 */
		bitmap_t *zmap;

	};

	/**
	 * @brief Minix File System Information
	 */
	extern struct minix_fs_info minix_fs;

	/*============================================================================*
	 * Block Interface                                                            *
	 *============================================================================*/

	/**
	 * @brief Allocates a disk block.
	 *
	 * @param sb   Target subperbloc.
	 * @param zmap Target zone map.
	 *
	 * @returns Upon successful completion, the number of the allocated
	 * block is returned. Upon failure, MINIX_BLOCK_NULL is returned
	 * instead.
	 */
	extern minix_block_t minix_block_alloc(
		const struct d_superblock *sb,
		bitmap_t *zmap
	);

	/**
	 * @brief Frees a direct disk block.
	 *
	 * @param sb  Target superblock.
	 * @param num Number of the target direct block.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int minix_block_free_direct(
		struct d_superblock *sb,
		bitmap_t *zmap,
		minix_block_t num
	);

	/**
	 * @brief Frees an indirect disk block.
	 *
	 * @param sb  Target superblock.
	 * @param num Number of the target indirect block.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int minix_block_free_indirect(
		struct d_superblock *sb,
		bitmap_t *zmap,
		minix_block_t num
	);

	/**
	 * @brief Frees an double indirect disk block.
	 *
	 * @param sb  Target superblock.
	 * @param num Number of the target double indirect block.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int minix_block_free_dindirect(
		struct d_superblock *sb,
		bitmap_t *zmap,
		minix_block_t num
	);

	/**
	 * @brief Frees a disk block.
	 *
	 * @param sb  Target superblock.
	 * @param num Number of the target block.
	 * @param lvl Level of indirection.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int minix_block_free(
		struct d_superblock *sb,
		bitmap_t *zmap,
		minix_block_t num,
		int lvl
	);

	/**
	 * @brief Maps a file byte offset in a block number.
	 *
	 * @param ip     Target inode.
	 * @param off    File offset.
	 * @param create Create offset?
	 *
	 * @returns Upon sucessful completion, the block number that is
	 * allocated for the file byte offset @p off is returned. Upon
	 * failure, MINIX_BLOCK_NULL is returned instead.
	 */
	extern minix_block_t minix_block_map(struct d_superblock *sb, bitmap_t *zmap, struct d_inode *ip, off_t off, int create);

	extern void minix_inode_write(struct d_superblock *sb, struct d_inode *ip, minix_ino_t num);
	extern struct d_inode *minix_inode_read(struct d_superblock *sb, struct d_inode *ip, minix_ino_t num);
	extern minix_ino_t minix_inode_alloc(struct d_superblock *sb, bitmap_t *imap, uint16_t mode, uint16_t uid, uint16_t gid);
	extern void minix_super_read(struct d_superblock *sb, bitmap_t *imap, bitmap_t *zmap);
	extern void minix_super_write(struct d_superblock *sb, bitmap_t *imap, bitmap_t *zmap);
	extern void minix_mkfs (minix_ino_t ninodes, minix_block_t nblocks, uint16_t uid, uint16_t gid);

#endif /* _MINIX_H_ */
