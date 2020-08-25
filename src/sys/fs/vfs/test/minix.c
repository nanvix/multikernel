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

#include <dev/ramdisk.h>
#include <nanvix/servers/vfs.h>
#include <nanvix/config.h>
#include <nanvix/dev.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/*============================================================================*
 * MINIX File System Tests                                                    *
 *============================================================================*/

/**
 * @brief API Test: Block Alloc/Free
 */
static void test_api_minix_block_alloc_free(void)
{
	minix_block_t num;

	uassert((
		num = minix_block_alloc(
			&fs_root.super->data,
			fs_root.super->bmap
		)) != MINIX_BLOCK_NULL
	);

	uassert(
		minix_block_free_direct(
			&fs_root.super->data,
			fs_root.super->bmap,
			num
		)  == 0
	);
}

/**
 * @brief API Test: Inode Alloc/Free
 */
static void test_api_minix_inode_alloc_free(void)
{
	minix_ino_t ino;

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&fs_root.super->data,
			fs_root.super->imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

	uassert((
		minix_inode_free(
			&fs_root.super->data,
			fs_root.super->imap,
			ino)
		) == 0
	);
}

/**
 * @brief API Test: Inode Read/Write
 */
static void test_api_minix_inode_read_write(void)
{
	minix_ino_t ino;
	struct d_inode inode;

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&fs_root.super->data,
			fs_root.super->imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_inode_read(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				&inode,
				ino)
			) == 0
		);

		uassert((
			minix_inode_write(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				&inode,
				ino)
			) == 0
		);

	uassert((
		minix_inode_free(
			&fs_root.super->data,
			fs_root.super->imap,
			ino)
		) == 0
	);
}

/**
 * @brief API Test: Directory Entry Add/Remove
 */
static void test_api_minix_dirent_add_remove(void)
{
	minix_ino_t ino;
	const char *filename = "test-file";

	uassert((
		ino = minix_inode_alloc(
			fs_root.dev,
			&fs_root.super->data,
			fs_root.super->imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_dirent_add(
				fs_root.dev,
				&fs_root.super->data,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				filename,
				ino)
			) == 0
		);

		uassert((
			minix_dirent_remove(
				fs_root.dev,
				&fs_root.super->data,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				filename)
			) == 0
		);

	uassert((
		minix_inode_free(
			&fs_root.super->data,
			fs_root.super->imap,
			ino)
		) == 0
	);
}

/**
 * @brief API Test: Directory Entry Search
 */
static void test_api_minix_dirent_search(void)
{
	minix_ino_t ino;
	const char *filename = "test-file";

	uassert((
		ino = minix_inode_alloc(
			fs_root.dev,
			&fs_root.super->data,
			fs_root.super->imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_dirent_add(
				fs_root.dev,
				&fs_root.super->data,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				filename,
				ino)
			) == 0
		);

			uassert((
				minix_dirent_search(
					fs_root.dev,
					&fs_root.super->data,
					fs_root.super->bmap,
					inode_disk_get(fs_root.root),
					filename,
					0)
				) >= 0
			);

		uassert((
			minix_dirent_remove(
				fs_root.dev,
				&fs_root.super->data,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				filename)
			) == 0
		);

	uassert((
		minix_inode_free(
			&fs_root.super->data,
			fs_root.super->imap,
			ino)
		) == 0
	);
}

/**
 * @brief Fault Injection Test: Invalid Alloc
 */
static void test_fault_minix_block_alloc_inval(void)
{
	uassert(
		minix_block_alloc(
			NULL,
			fs_root.super->bmap
		) == MINIX_BLOCK_NULL
	);

	uassert(
		minix_block_alloc(
			&fs_root.super->data,
			NULL
		) == MINIX_BLOCK_NULL
	);
}

/**
 * @brief Fault Injection Test: Invalid Free
 */
static void test_fault_minix_block_free_inval(void)
{
	minix_block_t num;

	uassert((
		num = minix_block_alloc(
			&fs_root.super->data,
			fs_root.super->bmap
		)) != MINIX_BLOCK_NULL
	);

		uassert(
			minix_block_free_direct(
				NULL,
				fs_root.super->bmap,
				num
			)  == -EINVAL
		);
		uassert(
			minix_block_free_direct(
				&fs_root.super->data,
				NULL,
				num
			)  == -EINVAL
		);
		uassert(
			minix_block_free_direct(
				&fs_root.super->data,
				fs_root.super->bmap,
				MINIX_BLOCK_NULL
			)  == -EINVAL
		);

	uassert(
		minix_block_free_direct(
			&fs_root.super->data,
			fs_root.super->bmap,
			num
		)  == 0
	);
}

/**
 * @brief Fault Injection Test: Invalid Superblock Read
 */
static void test_fault_minix_super_read_inval(void)
{
	struct d_superblock sb;
	bitmap_t *imap;
	bitmap_t *zmap;

	uassert(
		minix_super_read(
			-1,
			&sb,
			&zmap,
			&imap
		) == -EINVAL
	);

	uassert(
		minix_super_read(
			NANVIX_ROOT_DEV,
			NULL,
			&zmap,
			&imap
		) == -EINVAL
	);

	uassert(
		minix_super_read(
			NANVIX_ROOT_DEV,
			&sb,
			NULL,
			&imap
		) == -EINVAL
	);

	uassert(
		minix_super_read(
			NANVIX_ROOT_DEV,
			&sb,
			&zmap,
			NULL
		) == -EINVAL
	);
}

/**
 * @brief Fault Injection Test: Invalid Superblock Write
 */
static void test_fault_minix_super_write_inval(void)
{
	struct d_superblock sb;
	bitmap_t imap[MINIX_BLOCK_SIZE/sizeof(bitmap_t)];
	bitmap_t zmap[MINIX_BLOCK_SIZE/sizeof(bitmap_t)];

	uassert(
		minix_super_write(
			-1,
			&sb,
			zmap,
			imap
		) == -EINVAL
	);

	uassert(
		minix_super_write(
			NANVIX_ROOT_DEV,
			NULL,
			zmap,
			imap
		) == -EINVAL
	);

	uassert(
		minix_super_write(
			NANVIX_ROOT_DEV,
			&sb,
			NULL,
			imap
		) == -EINVAL
	);

	uassert(
		minix_super_write(
			NANVIX_ROOT_DEV,
			&sb,
			zmap,
			NULL
		) == -EINVAL
	);
}

/**
 * @brief Fault Injection Test: Invalid Inode Alloc
 */
static void test_fault_minix_inode_alloc_inval(void)
{
	uassert((
		minix_inode_alloc(
			-1,
			&fs_root.super->data,
			fs_root.super->imap,
			0, 0, 0)
		) == MINIX_INODE_NULL
	);
	uassert((
		minix_inode_alloc(
			NANVIX_ROOT_DEV,
			NULL,
			fs_root.super->imap,
			0, 0, 0)
		) == MINIX_INODE_NULL
	);
	uassert((
		minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&fs_root.super->data,
			NULL,
			0, 0, 0)
		) == MINIX_INODE_NULL
	);
}

/**
 * @brief Fault Injection Test: Invalid Inode Free
 */
static void test_fault_minix_inode_free_inval(void)
{
	minix_ino_t ino;

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&fs_root.super->data,
			fs_root.super->imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_inode_free(
				NULL,
				fs_root.super->imap,
				ino)
			) == -EINVAL
		);
		uassert((
			minix_inode_free(
				&fs_root.super->data,
				NULL,
				ino)
			) == -EINVAL
		);
		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				MINIX_INODE_NULL)
			) == -EINVAL
		);
		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				fs_root.super->data.s_ninodes + 1)
			) == -EINVAL
		);

	uassert((
		minix_inode_free(
			&fs_root.super->data,
			fs_root.super->imap,
			ino)
		) == 0
	);
}

/**
 * @brief Fault Injection Test: Invalid Inode Read
 */
static void test_fault_minix_inode_read_inval(void)
{
	minix_ino_t ino;
	struct d_inode inode;

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&fs_root.super->data,
			fs_root.super->imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_inode_read(
				-1,
				&fs_root.super->data,
				&inode,
				ino)
			) == -EAGAIN
		);

		uassert((
			minix_inode_read(
				NANVIX_ROOT_DEV,
				NULL,
				&inode,
				ino)
			) == -EINVAL
		);

		uassert((
			minix_inode_read(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				NULL,
				ino)
			) == -EINVAL
		);

		uassert((
			minix_inode_read(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				&inode,
				MINIX_INODE_NULL)
			) == -EINVAL
		);

		uassert((
			minix_inode_read(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				&inode,
				fs_root.super->data.s_ninodes + 1)
			) == -EINVAL
		);

	uassert((
		minix_inode_free(
			&fs_root.super->data,
			fs_root.super->imap,
			ino)
		) == 0
	);
}

/**
 * @brief Fault Injection Test: Invalid Inode Write
 */
static void test_fault_minix_inode_write_inval(void)
{
	minix_ino_t ino;
	struct d_inode inode;

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&fs_root.super->data,
			fs_root.super->imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_inode_write(
				-1,
				&fs_root.super->data,
				&inode,
				ino)
			) == -EAGAIN
		);
		uassert((
			minix_inode_write(
				NANVIX_ROOT_DEV,
				NULL,
				&inode,
				ino)
			) == -EINVAL
		);
		uassert((
			minix_inode_write(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				NULL,
				ino)
			) == -EINVAL
		);
		uassert((
			minix_inode_write(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				&inode,
				MINIX_INODE_NULL)
			) == -EINVAL
		);
		uassert((
			minix_inode_write(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				&inode,
				fs_root.super->data.s_ninodes + 1)
			) == -EINVAL
		);

	uassert((
		minix_inode_free(
			&fs_root.super->data,
			fs_root.super->imap,
			ino)
		) == 0
	);
}

/**
 * @brief Fault Injection Test: Invalid Directory Entry Add
 */
static void test_fault_minix_dirent_add_inval(void)
{
	minix_ino_t ino;
	const char *filename = "test-file";
	const char *longname =
		"i like hamburguers, with bacon, cheese, more bacon, and more cheese";

	uassert((
		ino = minix_inode_alloc(
			fs_root.dev,
			&fs_root.super->data,
			fs_root.super->imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_dirent_add(
				fs_root.dev,
				NULL,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				filename,
				ino)
			) == -EINVAL
		);

		uassert((
			minix_dirent_add(
				fs_root.dev,
				&fs_root.super->data,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				NULL,
				ino)
			) == -EINVAL
		);

		uassert((
			minix_dirent_add(
				fs_root.dev,
				&fs_root.super->data,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				longname,
				ino)
			) == -ENAMETOOLONG
		);

		uassert((
			minix_dirent_add(
				fs_root.dev,
				&fs_root.super->data,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				filename,
				MINIX_INODE_NULL)
			) == -EINVAL
		);

	uassert((
		minix_inode_free(
			&fs_root.super->data,
			fs_root.super->imap,
			ino)
		) == 0
	);
}

/**
 * @brief Fault Injection Test: Invalid Directory Entry Remove
 */
static void test_fault_minix_dirent_remove_inval(void)
{
	minix_ino_t ino;
	const char *filename = "test-file";
	const char *longname =
		"i like hamburguers, with bacon, cheese, more bacon, and more cheese";

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&fs_root.super->data,
			fs_root.super->imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_dirent_add(
				fs_root.dev,
				&fs_root.super->data,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				filename,
				ino)
			) == 0
		);

			uassert((
				minix_dirent_remove(
					fs_root.dev,
					NULL,
					fs_root.super->bmap,
					inode_disk_get(fs_root.root),
					filename)
				) == -EINVAL
			);

			uassert((
				minix_dirent_remove(
					fs_root.dev,
					&fs_root.super->data,
					fs_root.super->bmap,
					NULL,
					filename)
				) == -EINVAL
			);

			uassert((
				minix_dirent_remove(
					fs_root.dev,
					&fs_root.super->data,
					fs_root.super->bmap,
					inode_disk_get(fs_root.root),
					NULL)
				) == -EINVAL
			);

			uassert((
				minix_dirent_remove(
					fs_root.dev,
					&fs_root.super->data,
					fs_root.super->bmap,
					inode_disk_get(fs_root.root),
					longname)
				) == -ENAMETOOLONG
			);

		uassert((
			minix_dirent_remove(
				fs_root.dev,
				&fs_root.super->data,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				filename)
			) == 0
		);

	uassert((
		minix_inode_free(
			&fs_root.super->data,
			fs_root.super->imap,
			ino)
		) == 0
	);
}

/**
 * @brief Fault Injection Test: Invalid Directory Entry Search
 */
static void test_fault_minix_dirent_search_inval(void)
{
	minix_ino_t ino;
	const char *filename = "test-file";
	const char *longname =
		"i like hamburguers, with bacon, cheese, more bacon, and more cheese";

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&fs_root.super->data,
			fs_root.super->imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_dirent_add(
				fs_root.dev,
				&fs_root.super->data,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				filename,
				ino)
			) == 0
		);

			uassert((
				minix_dirent_search(
					fs_root.dev,
					&fs_root.super->data,
					fs_root.super->bmap,
					NULL,
					filename,
					0)
				) == -EINVAL
			);

			uassert((
				minix_dirent_search(
					fs_root.dev,
					&fs_root.super->data,
					fs_root.super->bmap,
					inode_disk_get(fs_root.root),
					NULL,
					0)
				) == -EINVAL
			);

			uassert((
				minix_dirent_search(
					fs_root.dev,
					&fs_root.super->data,
					fs_root.super->bmap,
					inode_disk_get(fs_root.root),
					longname,
					0)
				) == -ENAMETOOLONG
			);

		uassert((
			minix_dirent_remove(
				fs_root.dev,
				&fs_root.super->data,
				fs_root.super->bmap,
				inode_disk_get(fs_root.root),
				filename)
			) == 0
		);

	uassert((
		minix_inode_free(
			&fs_root.super->data,
			fs_root.super->imap,
			ino)
		) == 0
	);
}

/**
 * @brief Sress Test: Block Alloc/Free
 */
static void test_stress_minix_block_alloc_free1(void)
{
	minix_block_t blocks[NANVIX_DISK_SIZE/MINIX_BLOCK_SIZE];

	for (unsigned i = 1; i < NANVIX_DISK_SIZE/MINIX_BLOCK_SIZE; i++)
	{
		uassert((
			blocks[i] = minix_block_alloc(
				&fs_root.super->data,
				fs_root.super->bmap
			)) != MINIX_BLOCK_NULL
		);

		uassert(
			minix_block_free_direct(
				&fs_root.super->data,
				fs_root.super->bmap,
				blocks[i]
			)  == 0
		);
	}
}

/**
 * @brief Sress Test: Block Alloc/Free
 */
static void test_stress_minix_block_alloc_free2(void)
{
	minix_block_t blocks[NANVIX_DISK_SIZE/MINIX_BLOCK_SIZE];

	for (unsigned i = 1; i < NANVIX_DISK_SIZE/MINIX_BLOCK_SIZE; i++)
	{
		uassert((
			blocks[i] = minix_block_alloc(
				&fs_root.super->data,
				fs_root.super->bmap
			)) != MINIX_BLOCK_NULL
		);
	}

	for (unsigned i = 1; i < NANVIX_DISK_SIZE/MINIX_BLOCK_SIZE; i++)
	{
		uassert(
			minix_block_free_direct(
				&fs_root.super->data,
				fs_root.super->bmap,
				blocks[i]
			)  == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Alloc/Free Sequential One Step
 */
static void test_stress_minix_inode_alloc_free1(void)
{
	minix_ino_t inos[NANVIX_NR_INODES];

	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i++)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Alloc/Free Sequential Two Steps
 */
static void test_stress_minix_inode_alloc_free2(void)
{
	minix_ino_t inos[NANVIX_NR_INODES];

	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i++)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);
	}

	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i++)
	{

		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Read/Write Sequential One Step
 */
static void test_stress_minix_inode_read_write1(void)
{
	minix_ino_t inos[NANVIX_NR_INODES];
	struct d_inode inode;

	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i++)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

			uassert((
				minix_inode_read(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);

			uassert((
				minix_inode_write(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);

		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Read/Write Sequential Two Steps
 */
static void test_stress_minix_inode_read_write2(void)
{
	minix_ino_t inos[NANVIX_NR_INODES];
	struct d_inode inode;

	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i++)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

	}

		for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i++)
		{
			uassert((
				minix_inode_read(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);
		}

		for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i++)
		{
			uassert((
				minix_inode_write(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);
		}

	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i++)
	{

		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Alloc/Free Interleaved One Step
 */
static void test_stress_minix_inode_alloc_free_interleaved1(void)
{
	minix_ino_t inos[NANVIX_NR_INODES];

	/* Even inodes*/
	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i+=2)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}

	/* Odd inodes*/
	for (minix_ino_t i = 3; i < fs_root.super->data.s_ninodes; i+=2)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Alloc/Free Interleaved Two Steps
 */
static void test_stress_minix_inode_alloc_free_interleaved2(void)
{
	minix_ino_t inos[NANVIX_NR_INODES];

	/* Even inodes*/
	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i+=2)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);
	}

	/* Odd inodes*/
	for (minix_ino_t i = 3; i < fs_root.super->data.s_ninodes; i+=2)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);
	}

	/* Even inodes*/
	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i+=2)
	{
		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}

	/* Odd inodes*/
	for (minix_ino_t i = 3; i < fs_root.super->data.s_ninodes; i+=2)
	{
		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Read/Write Interleaved One Step
 */
static void test_stress_minix_inode_read_write_interleaved1(void)
{
	minix_ino_t inos[NANVIX_NR_INODES];
	struct d_inode inode;

	/*Even inodes*/
	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i+=2)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

			uassert((
				minix_inode_read(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);

			uassert((
				minix_inode_write(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);

		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}

	/*Odd inodes*/
	for (minix_ino_t i = 3; i < fs_root.super->data.s_ninodes; i+=2)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

			uassert((
				minix_inode_read(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);

			uassert((
				minix_inode_write(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);

		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Read/Write Interleaved Two Steps
 */
static void test_stress_minix_inode_read_write_interleaved2(void)
{
	minix_ino_t inos[NANVIX_NR_INODES];
	struct d_inode inode;

	/*Even inodes*/
	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i+=2)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

	}

	/*Odd inodes*/
	for (minix_ino_t i = 3; i < fs_root.super->data.s_ninodes; i+=2)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&fs_root.super->data,
				fs_root.super->imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

	}

		/*Even inodes*/
		for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i+=2)
		{
			uassert((
				minix_inode_read(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);
		}

		/*Odd inodes*/
		for (minix_ino_t i = 3; i < fs_root.super->data.s_ninodes; i+=2)
		{
			uassert((
				minix_inode_read(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);
		}

		/*Even inodes*/
		for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i+=2)
		{
			uassert((
				minix_inode_write(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);
		}

		/*Odd inodes*/
		for (minix_ino_t i = 3; i < fs_root.super->data.s_ninodes; i+=2)
		{
			uassert((
				minix_inode_write(
					NANVIX_ROOT_DEV,
					&fs_root.super->data,
					&inode,
					inos[i])
				) == 0
			);
		}

	/*Even inodes*/
	for (minix_ino_t i = 2; i < fs_root.super->data.s_ninodes; i+=2)
	{

		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}

	/*Odd inodes*/
	for (minix_ino_t i = 3; i < fs_root.super->data.s_ninodes; i+=2)
	{

		uassert((
			minix_inode_free(
				&fs_root.super->data,
				fs_root.super->imap,
				inos[i])
			) == 0
		);
	}
}

/**
 * @brief MINIX File System Tests
 */
static struct
{
	void (*func)(void); /**< Test Function */
	const char *name;   /**< Test Name     */
} minix_tests[] = {
	{ test_api_minix_block_alloc_free,                "[minix][api] block alloc/free                 " },
	{ test_api_minix_inode_alloc_free,                "[minix][api] inode alloc/free                 " },
	{ test_api_minix_inode_read_write,                "[minix][api] inode read/write                 " },
	{ test_api_minix_dirent_add_remove,               "[minix][api] dirent add/remove                " },
	{ test_api_minix_dirent_search,                   "[minix][api] dirent search                    " },
	{ test_fault_minix_block_alloc_inval,             "[minix][fault] block alloc inval              " },
	{ test_fault_minix_block_free_inval,              "[minix][fault] block free inval               " },
	{ test_fault_minix_super_read_inval,              "[minix][fault] superblock read inval          " },
	{ test_fault_minix_super_write_inval,             "[minix][fault] superblock write inval         " },
	{ test_fault_minix_inode_alloc_inval,             "[minix][fault] inode alloc inval              " },
	{ test_fault_minix_inode_free_inval,              "[minix][fault] inode free inval               " },
	{ test_fault_minix_inode_read_inval,              "[minix][fault] inode read inval               " },
	{ test_fault_minix_inode_write_inval,             "[minix][fault] inode write inval              " },
	{ test_fault_minix_dirent_add_inval,              "[minix][fault] dirent add inval               " },
	{ test_fault_minix_dirent_remove_inval,           "[minix][fault] dirent remove inval            " },
	{ test_fault_minix_dirent_search_inval,           "[minix][fault] dirent search inval            " },
	{ test_stress_minix_block_alloc_free1,            "[minix][stress] block alloc/free 1            " },
	{ test_stress_minix_block_alloc_free2,            "[minix][stress] block alloc/free 2            " },
	{ test_stress_minix_inode_alloc_free1,            "[minix][stress] inode alloc/free 1            " },
	{ test_stress_minix_inode_alloc_free2,            "[minix][stress] inode alloc/free 2            " },
	{ test_stress_minix_inode_read_write1,            "[minix][stress] inode read/write 1            " },
	{ test_stress_minix_inode_read_write2,            "[minix][stress] inode read/write 2            " },
	{ test_stress_minix_inode_alloc_free_interleaved1,"[minix][stress] inode alloc/free interleaved 1" },
	{ test_stress_minix_inode_alloc_free_interleaved2,"[minix][stress] inode alloc/free interleaved 2" },
	{ test_stress_minix_inode_read_write_interleaved1,"[minix][stress] inode read/write interleaved 1" },
	{ test_stress_minix_inode_read_write_interleaved2,"[minix][stress] inode read/write interleaved 2" },
	{ NULL,                                 NULL                                    },
};

/**
 * @brief Runs regression tests on RAM Disk.
 */
void test_minix(void)
{
	for (int i = 0; minix_tests[i].func != NULL; i++)
	{
		minix_tests[i].func();

		uprintf("[nanvix][vfs]%s passed", minix_tests[i].name);
	}
}
