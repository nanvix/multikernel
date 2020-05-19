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
	#include <nanvix/ulib.h>
	#include <posix/sys/types.h>

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
		 * @brief Underlying Device
		 */
		dev_t dev;

		/**
		 * @brief Inode Number of Root Directory
		 */
		minix_ino_t root_ino;

		/**
		 * @brief Root Directory
		 */
		struct d_inode root;

		/**
		 * @brief Superblock
		 */
		struct d_superblock super;

		/**
		 * @brief Inode Map
		 */
		bitmap_t *imap;

		/**
		 * @brief Zone Map
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
	 * @brief Allocates a  file system block.
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
	 * @brief Frees a direct file system block.
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
	 * @brief Frees an indirect  file system block.
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
	 * @brief Frees an double indirect  file system block.
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
	 * @brief Frees a file system block.
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
	extern minix_block_t minix_block_map(
		struct d_superblock *sb,
		bitmap_t *zmap,
		struct d_inode *ip,
		off_t off,
		int create
	);

/*============================================================================*
 * Superblock Interface                                                       *
 *============================================================================*/

	/**
	 * @brief Reads the superblock of a MINIX file system.
	 *
	 * @param dev  Target disk device.
	 * @param sb   Target location to store the superblock.
	 * @param zmap Target location to store zone map.
	 * @param imap Target location to store the inode map
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int minix_super_read(
		dev_t dev,
		struct d_superblock *sb,
		bitmap_t *zmap,
		bitmap_t *imap
	);

	/**
	 * @brief Writes the superblock of a MINIX file system.
	 *
	 * @param dev  Target disk device.
	 * @param sb   Target superblock.
	 * @param zmap Target zone map.
	 * @param imap Target inode map
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int minix_super_write(
		dev_t dev,
		const struct d_superblock *sb,
		const bitmap_t *zmap,
		const bitmap_t *imap
	);

/*============================================================================*
 * Inode Interface                                                            *
 *============================================================================*/

	/**
	 * @brief Allocates an inode.
	 *
	 * @param dev  Target device.
	 * @param sb   Target superblock.
	 * @param imap Target inode map.
	 * @param mode Access mode.
	 * @param uid  User ID.
	 * @param gid  User group ID.
	 *
	 * @returns Upon successful completion, the number of the allocated
	 * inode is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern minix_ino_t minix_inode_alloc(
		dev_t dev,
		struct d_superblock *sb,
		bitmap_t *imap,
		minix_mode_t mode,
		minix_uid_t uid,
		minix_gid_t gid
	);

	/**
	 * @brief Frees an inode.
	 *
	 * @param sb   Target superblock.
	 * @param imap Target inode map.
	 * @param num  Number of the target inode.
	 *
	 * @returns Upon successful completion, the number of the allocated
	 * inode is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern int minix_inode_free(
		struct d_superblock *sb,
		bitmap_t *imap,
		minix_ino_t num
	);

	/**
	 * @brief Reads an inode from the disk.
	 *
	 * @param dev Target device.
	 * @param sb  Target superblock.
	 * @param ip  Target inode.
	 * @param num Number of the target inode.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int minix_inode_read(
		dev_t dev,
		struct d_superblock *sb,
		struct d_inode *ip,
		minix_ino_t num
	);

	/**
	 * @brief Writes an inode to the disk.
	 *
	 * @param dev Target device.
	 * @param sb  Target superblock.
	 * @param ip  Target inode.
	 * @param num Number of the target inode.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int minix_inode_write(
		dev_t dev,
		struct d_superblock *sb,
		struct d_inode *ip,
		minix_ino_t num
	);

/*============================================================================*
 * File System Interface                                                      *
 *============================================================================*/

	/**
	 * @brief Adds an entry in a directory.
	 *
	 * @param dip  Target directory.
	 * @param name Name of the entry.
	 * @param num  Number of linked inode.
	 *
	 * @returns upon successful completion, zero is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	extern int minix_dirent_add(
		struct d_inode *dip,
		const char *name,
		minix_ino_t num
	);

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
	extern int minix_dirent_remove(
		struct d_inode *dip,
		const char *name
	);

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
	 * @returns iUpon successful completion, the offset where the directory
	 * entry is located is returned. Upon failure, a negative error code is
	 * returned instead..
	 */
	extern off_t minix_dirent_search(
		struct d_inode *dip,
		const char *name,
		int create
	);

	/**
	 * @brief Makes a MINIX file system.
	 *
	 * @param dev      Target device.
	 * @param ninodes  Number of inodes.
	 * @param nblocks  Number of blocks.
	 * @param uid      User ID.
	 * @param gid      User group ID.
	 *
	 * @note @p diskfile must refer to a valid file.
	 * @note @p ninodes must be valid.
	 * @note @p nblocks must be valid.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int minix_mkfs(
		dev_t dev,
		minix_ino_t ninodes,
		minix_block_t nblocks,
		minix_uid_t uid,
		minix_gid_t gid
	);

#endif /* _MINIX_H_ */
