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

#ifndef FS_MINIX_H_
#define FS_MINIX_H_

	#include <posix/stdint.h>

/*============================================================================*
 * Block Information                                                          *
 *============================================================================*/

	/**
	 * @brief Log 2 of block size.
	 */
	#define MINIX_BLOCK_SIZE_LOG2 10

	/**
	 * @brief Block size (in bytes).
	 */
	#define MINIX_BLOCK_SIZE (1 << MINIX_BLOCK_SIZE_LOG2)

	/**
	 * @brief Null block.
	 */
	#define MINIX_BLOCK_NULL 0

	/**
	 * @brief Block Number
	 */
	typedef uint16_t minix_block_t;

/*============================================================================*
 * Superblock Information                                                     *
 *============================================================================*/

	/**
 	 * @brief Superblock magic number.
 	 */
 	#define MINIX_SUPER_MAGIC 0x137f

	/**
	 * @brief In-disk superblock.
	 */
	struct d_superblock
	{
		uint16_t s_ninodes;          /**< Number of inodes.           */
		uint16_t s_nblocks;          /**< Number of blocks.           */
		uint16_t s_imap_nblocks;     /**< Number of inode map blocks. */
		uint16_t s_bmap_nblocks;     /**< Number of block map blocks. */
		uint16_t s_first_data_block; /**< Unused.                     */
		uint16_t unused1;            /**< Unused.                     */
		uint32_t s_max_size;         /**< Maximum file size.          */
		uint16_t s_magic;            /**< Magic number.               */
	} __attribute__((packed));

/*============================================================================*
 * Inode Information                                                          *
 *============================================================================*/

	/**
	 * @brief Null inode.
	 */
	#define MINIX_INODE_NULL 0

	/**
	 * @brief Root inode.
	 */
	#define MINIX_INODE_ROOT 1

	/**
	 * @name Number of Zones
	 *
	 * @details Number of zones in an #inode.
	 */
	/**@{*/
	#define MINIX_NR_ZONES_DIRECT 7 /**< Number of direct zones.          */
	#define MINIX_NR_ZONES_SINGLE 1 /**< Number of single indirect zones. */
	#define MINIX_NR_ZONES_DOUBLE 1 /**< Number of double indirect zones. */
	#define MINIX_NR_ZONES        9 /**< Total of zones.                  */
	/**@}*/

	/**
	 * @name Zone Index
	 *
	 * @details Index of zones in an #inode.
	 */
	/**@{*/
	#define MINIX_ZONE_DIRECT                                          0  /**< Direct zone.     */
	#define MINIX_ZONE_SINGLE                     (MINIX_NR_ZONES_DIRECT) /**< Single indirect. */
	#define MINIX_ZONE_DOUBLE (MINIX_ZONE_SINGLE + MINIX_NR_ZONES_SINGLE) /**< Double indirect. */
	/**@}*/

	/**
	 * @name Zone Dimensions
	 *
	 * @details Dimension of zones.
	 */
	/**@{*/

	/** Number of zones in a direct zone. */
	#define MINIX_NR_DIRECT 1

	/** Number of zones in a single indirect zone. */
	#define MINIX_NR_SINGLE (MINIX_BLOCK_SIZE/sizeof(minix_block_t))

	/** Number of zones in a double indirect zone. */
	#define MINIX_NR_DOUBLE ((MINIX_BLOCK_SIZE/sizeof(minix_block_t))*MINIX_NR_SINGLE)

	/**@}*/

	/**
	 * @brief Disk inode.
	 */
	struct d_inode
	{
		uint16_t i_mode;                  /**< Access permissions.                  */
		uint16_t i_uid;                   /**< User id of the file's owner          */
		uint32_t i_size;                  /**< File size (in bytes).                */
		uint32_t i_time;                  /**< Time when the file was last accessed.*/
		uint8_t i_gid;                    /**< Group number of owner user.          */
		uint8_t i_nlinks;                 /**< Number of links to the file.         */
		uint16_t i_zones[MINIX_NR_ZONES]; /**< Zone numbers.                        */
	} __attribute__((packed));

	/**
	 * @brief I-Node Number
	 */
	typedef uint16_t minix_ino_t;

	/**
	 * @brief Access Mode
	 */
	typedef uint16_t minix_mode_t;

	/**
	 * @brief User
	 */
	typedef uint16_t minix_uid_t;

	/**
	 * @brief User Group
	 */
	typedef uint16_t minix_gid_t;

/*============================================================================*
 * Directory Entry Information                                                *
 *============================================================================*/

	/**
	 * @brief Maximum name on a Minix file system.
	 */
	#define MINIX_NAME_MAX 14

	/*
	 * Directory entry.
	 */
	struct d_dirent
	{
		uint16_t d_ino;              /**< File serial number. */
		char d_name[MINIX_NAME_MAX]; /**< Name of entry.      */
	} __attribute__((packed));

#endif /* FS_MINIX_H_ */

