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

#include <dev/ramdisk.h>
#include <nanvix/config.h>
#include <nanvix/dev.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include "../include/const.h"
#include "../include/bcache.h"
#include "../include/minix.h"

/**
 * @brief Buffer for Read/Write Tests
 */
static char data[NANVIX_FS_BLOCK_SIZE];

/*============================================================================*
 * RAM Disk Tests                                                             *
 *============================================================================*/

/**
 * @brief API Test: Read/Write to RAM Disk
 */
static void ramdisk_api_read_write(void)
{
	umemset(data, 1, sizeof(data));

	for (unsigned minor = 0; minor < NANVIX_NR_RAMDISKS; minor++)
	{
		ramdisk_write(minor, data, sizeof(data), 0);
		ramdisk_read(minor, data, sizeof(data), 0);

		/* Checksum. */
		for (size_t i = 0; i < sizeof(data); i++)
			uassert(data[i] == 1);
	}
}

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void ramdisk_fault_read_inval(void)
{
	uassert(ramdisk_read(-1, data, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_read(NANVIX_NR_RAMDISKS, data, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_read(0, NULL, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_read(0, data, -1, 0) == -EINVAL);
	uassert(ramdisk_read(0, data, NANVIX_RAMDISK_SIZE + 1, 0) == -EINVAL);
	uassert(ramdisk_read(0, data, NANVIX_RAMDISK_SIZE, 1) == -EINVAL);
	uassert(ramdisk_read(0, data, NANVIX_RAMDISK_SIZE, -1) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Invalid Write
 */
static void ramdisk_fault_write_inval(void)
{
	uassert(ramdisk_write(-1, data, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_write(NANVIX_NR_RAMDISKS, data, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_write(0, NULL, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_write(0, data, -1, 0) == -EINVAL);
	uassert(ramdisk_write(0, data, NANVIX_RAMDISK_SIZE + 1, 0) == -EINVAL);
	uassert(ramdisk_write(0, data, NANVIX_RAMDISK_SIZE, 1) == -EINVAL);
	uassert(ramdisk_write(0, data, NANVIX_RAMDISK_SIZE, -1) == -EINVAL);
}

/**
 * @brief Stress Test: Read/Write to RAM Disk
 */
static void ramdisk_stress_read_write(void)
{
	umemset(data, 1, sizeof(data));

	for (unsigned minor = 0; minor < NANVIX_NR_RAMDISKS; minor++)
	{
		for (off_t off = 0; off < NANVIX_RAMDISK_SIZE; off += sizeof(data))
		{
			ramdisk_write(minor, data, sizeof(data), 0);
			ramdisk_read(minor, data, sizeof(data), 0);

		/* Checksum. */
		for (size_t i = 0; i < sizeof(data); i++)
			uassert(data[i] == 1);
		}
	}
}

/**
 * @brief RAM Disk Tests
 */
static struct
{
	void (*func)(void); /**< Test Function */
	const char *name;   /**< Test Name     */
} ramdisk_tests[] = {
	{ ramdisk_api_read_write,    "[ramdisk][api]    read/write"    },
	{ ramdisk_fault_read_inval,  "[ramdisk][fault]  invalid read"  },
	{ ramdisk_fault_write_inval, "[ramdisk][fault]  invalid write" },
	{ ramdisk_stress_read_write, "[ramdisk][stress] read/write"    },
	{ NULL,                       NULL                             },
};

/**
 * @brief Runs regression tests on RAM Disk.
 */
static void ramdisk_test(void)
{
	for (int i = 0; ramdisk_tests[i].func != NULL; i++)
	{
		ramdisk_tests[i].func();

		uprintf("[nanvix][vfs] %s passed", ramdisk_tests[i].name);
	}
}

/*============================================================================*
 * Block Cache Tests                                                          *
 *============================================================================*/

/**
 * @brief API Test: Block Read/Release
 */
static void bcache_api_bread_brelse(void)
{
	struct buffer *buf;

	uassert((buf = bread(0, 0)) != NULL);
	uassert(brelse(buf) == 0);
}

/**
 * @brief API Test: Block Read/Write
 */
static void bcache_api_bread_bwrite(void)
{
	struct buffer *buf;

	/* Write data. */
	uassert((buf = bread(0, 0)) != NULL);
	umemset(buffer_get_data(buf), 1, NANVIX_FS_BLOCK_SIZE);
	uassert(bwrite(buf) == 0);

	/* Checksum. */
	uassert((buf = bread(0, 0)) != NULL);
	for (size_t i = 0; i < NANVIX_FS_BLOCK_SIZE; i++)
		uassert(((char *)buffer_get_data(buf))[i] == 1);
	uassert(brelse(buf) == 0);
}

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void bcache_fault_bread_inval(void)
{
	uassert(bread(-1, 0) == NULL);
	uassert(bread(NANVIX_NR_RAMDISKS, 0) == NULL);
	uassert(bread(0, -1) == NULL);
	uassert(bread(0, NANVIX_DISK_SIZE/NANVIX_FS_BLOCK_SIZE) == NULL);
}

/**
 * @brief Fault Injection Test: Invalid Release
 */
static void bcache_fault_brelse_inval(void)
{
	uassert(brelse(NULL) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Invalid Write
 */
static void bcache_fault_bwrite_inval(void)
{
	uassert(bwrite(NULL) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Bad Release
 */
static void bcache_fault_brelse_bad(void)
{
	int buf;
	struct buffer *bufp;

	uassert(brelse((struct buffer *)&buf) == -EINVAL);

	uassert((bufp = bread(0, 0)) != NULL);
	uassert(brelse(bufp) == 0);
	uassert(brelse(bufp) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Bad Write
 */
static void bcache_fault_bwrite_bad(void)
{
	int buf;
	struct buffer *bufp;

	uassert(brelse((struct buffer *)&buf) == -EINVAL);

	uassert((bufp = bread(0, 0)) != NULL);
	uassert(bwrite(bufp) == 0);
	uassert(bwrite(bufp) == -EINVAL);
}

/**
 * @brief Stress Test: Block Read/Release
 */
static void bcache_stress_bread_brelse(void)
{
	struct buffer *buf;

	for (block_t blk = 0; blk < NANVIX_DISK_SIZE/NANVIX_FS_BLOCK_SIZE; blk++)
	{
		uassert((buf = bread(0, blk)) != NULL);
		uassert(brelse(buf) == 0);
	}
}

/**
 * @brief Stress Test: Block Read/Write
 */
static void bcache_stress_bread_bwrite(void)
{
	struct buffer *buf;

	for (block_t blk = 0; blk < NANVIX_DISK_SIZE/NANVIX_FS_BLOCK_SIZE; blk++)
	{
		/* Write data. */
		uassert((buf = bread(0, blk)) != NULL);
		umemset(buffer_get_data(buf), 1, NANVIX_FS_BLOCK_SIZE);
		uassert(bwrite(buf) == 0);

		/* Checksum. */
		uassert((buf = bread(0, blk)) != NULL);
		for (size_t i = 0; i < NANVIX_FS_BLOCK_SIZE; i++)
			uassert(((char *)buffer_get_data(buf))[i] == 1);
		uassert(brelse(buf) == 0);
	}
}

/**
 * @brief Block Cache Tests
 */
static struct
{
	void (*func)(void); /**< Test Function */
	const char *name;   /**< Test Name     */
} bcache_tests[] = {
	{ bcache_api_bread_brelse,    "[bcache][api] bread/brelse     " },
	{ bcache_api_bread_bwrite,    "[bcache][api] bread/bwrite     " },
	{ bcache_fault_bread_inval,   "[bcache][fault] invalid bread  " },
	{ bcache_fault_brelse_inval,  "[bcache][fault] invalid brelse " },
	{ bcache_fault_bwrite_inval,  "[bcache][fault] invalid bwrite " },
	{ bcache_fault_brelse_bad,    "[bcache][fault] bad brelse     " },
	{ bcache_fault_bwrite_bad,    "[bcache][fault] bad bwrite     " },
	{ bcache_stress_bread_brelse, "[bcache][stress] bread/brelse  " },
	{ bcache_stress_bread_bwrite, "[bcache][stress] bread/bwrite  " },
	{ NULL,                        NULL                             },
};

/**
 * @brief Runs regression tests on Block Cache
 */
static void bcache_test(void)
{
	for (int i = 0; bcache_tests[i].func != NULL; i++)
	{
		bcache_tests[i].func();

		uprintf("[nanvix][vfs] %s passed", bcache_tests[i].name);
	}
}

/*============================================================================*
 * MINIX File System Tests                                                    *
 *============================================================================*/

/**
 * @brief API Test: Block Alloc/Free
 */
static void minix_api_block_alloc_free(void)
{
	minix_block_t num;

	uassert((
		num = minix_block_alloc(
			&minix_fs.super,
			minix_fs.zmap
		)) != MINIX_BLOCK_NULL
	);

	uassert(
		minix_block_free_direct(
			&minix_fs.super,
			minix_fs.zmap,
			num
		)  == 0
	);
}

/**
 * @brief API Test: Inode Alloc/Free
 */
static void minix_api_inode_alloc_free(void)
{
	minix_ino_t ino;

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&minix_fs.super,
			minix_fs.imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

	uassert((
		minix_inode_free(
			&minix_fs.super,
			minix_fs.imap,
			ino)
		) == 0
	);
}

/**
 * @brief API Test: Inode Read/Write
 */
static void minix_api_inode_read_write(void)
{
	minix_ino_t ino;
	struct d_inode inode;

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&minix_fs.super,
			minix_fs.imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_inode_read(
				NANVIX_ROOT_DEV,
				&minix_fs.super,
				&inode,
				ino)
			) == 0
		);

		uassert((
			minix_inode_write(
				NANVIX_ROOT_DEV,
				&minix_fs.super,
				&inode,
				ino)
			) == 0
		);

	uassert((
		minix_inode_free(
			&minix_fs.super,
			minix_fs.imap,
			ino)
		) == 0
	);
}

/**
 * @brief Fault Injection Test: Invalid Alloc
 */
static void minix_fault_block_alloc_inval(void)
{
	uassert(
		minix_block_alloc(
			NULL,
			minix_fs.zmap
		) == MINIX_BLOCK_NULL
	);

	uassert(
		minix_block_alloc(
			&minix_fs.super,
			NULL
		) == MINIX_BLOCK_NULL
	);
}

/**
 * @brief Fault Injection Test: Invalid Free
 */
static void minix_fault_block_free_inval(void)
{
	minix_block_t num;

	uassert((
		num = minix_block_alloc(
			&minix_fs.super,
			minix_fs.zmap
		)) != MINIX_BLOCK_NULL
	);

		uassert(
			minix_block_free_direct(
				NULL,
				minix_fs.zmap,
				num
			)  == -EINVAL
		);
		uassert(
			minix_block_free_direct(
				&minix_fs.super,
				NULL,
				num
			)  == -EINVAL
		);
		uassert(
			minix_block_free_direct(
				&minix_fs.super,
				minix_fs.zmap,
				MINIX_BLOCK_NULL
			)  == -EINVAL
		);

	uassert(
		minix_block_free_direct(
			&minix_fs.super,
			minix_fs.zmap,
			num
		)  == 0
	);
}

/**
 * @brief Fault Injection Test: Invalid Superblock Read
 */
static void minix_fault_super_read_inval(void)
{
	struct d_superblock sb;
	bitmap_t imap[MINIX_BLOCK_SIZE/sizeof(bitmap_t)];
	bitmap_t zmap[MINIX_BLOCK_SIZE/sizeof(bitmap_t)];

	uassert(
		minix_super_read(
			-1,
			&sb,
			zmap,
			imap
		) == -EINVAL
	);

	uassert(
		minix_super_read(
			NANVIX_ROOT_DEV,
			NULL,
			zmap,
			imap
		) == -EINVAL
	);

	uassert(
		minix_super_read(
			NANVIX_ROOT_DEV,
			&sb,
			NULL,
			imap
		) == -EINVAL
	);

	uassert(
		minix_super_read(
			NANVIX_ROOT_DEV,
			&sb,
			zmap,
			NULL
		) == -EINVAL
	);
}

/**
 * @brief Fault Injection Test: Invalid Superblock Write
 */
static void minix_fault_super_write_inval(void)
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
static void minix_fault_inode_alloc_inval(void)
{
	uassert((
		minix_inode_alloc(
			-1,
			&minix_fs.super,
			minix_fs.imap,
			0, 0, 0)
		) == MINIX_INODE_NULL
	);
	uassert((
		minix_inode_alloc(
			NANVIX_ROOT_DEV,
			NULL,
			minix_fs.imap,
			0, 0, 0)
		) == MINIX_INODE_NULL
	);
	uassert((
		minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&minix_fs.super,
			NULL,
			0, 0, 0)
		) == MINIX_INODE_NULL
	);
}

/**
 * @brief Fault Injection Test: Invalid Inode Free
 */
static void minix_fault_inode_free_inval(void)
{
	minix_ino_t ino;

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&minix_fs.super,
			minix_fs.imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_inode_free(
				NULL,
				minix_fs.imap,
				ino)
			) == -EINVAL
		);
		uassert((
			minix_inode_free(
				&minix_fs.super,
				NULL,
				ino)
			) == -EINVAL
		);
		uassert((
			minix_inode_free(
				&minix_fs.super,
				minix_fs.imap,
				MINIX_INODE_NULL)
			) == -EINVAL
		);
		uassert((
			minix_inode_free(
				&minix_fs.super,
				minix_fs.imap,
				minix_fs.super.s_ninodes + 1)
			) == -EINVAL
		);

	uassert((
		minix_inode_free(
			&minix_fs.super,
			minix_fs.imap,
			ino)
		) == 0
	);
}

/**
 * @brief Fault Injection Test: Invalid Inode Read
 */
static void minix_fault_inode_read_inval(void)
{
	minix_ino_t ino;
	struct d_inode inode;

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&minix_fs.super,
			minix_fs.imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_inode_read(
				-1,
				&minix_fs.super,
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
				&minix_fs.super,
				NULL,
				ino)
			) == -EINVAL
		);

		uassert((
			minix_inode_read(
				NANVIX_ROOT_DEV,
				&minix_fs.super,
				&inode,
				MINIX_INODE_NULL)
			) == -EINVAL
		);

		uassert((
			minix_inode_read(
				NANVIX_ROOT_DEV,
				&minix_fs.super,
				&inode,
				minix_fs.super.s_ninodes + 1)
			) == -EINVAL
		);

	uassert((
		minix_inode_free(
			&minix_fs.super,
			minix_fs.imap,
			ino)
		) == 0
	);
}

/**
 * @brief Fault Injection Test: Invalid Inode Write
 */
static void minix_fault_inode_write_inval(void)
{
	minix_ino_t ino;
	struct d_inode inode;

	uassert((
		ino = minix_inode_alloc(
			NANVIX_ROOT_DEV,
			&minix_fs.super,
			minix_fs.imap,
			0, 0, 0)
		) != MINIX_INODE_NULL
	);

		uassert((
			minix_inode_write(
				-1,
				&minix_fs.super,
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
				&minix_fs.super,
				NULL,
				ino)
			) == -EINVAL
		);
		uassert((
			minix_inode_write(
				NANVIX_ROOT_DEV,
				&minix_fs.super,
				&inode,
				MINIX_INODE_NULL)
			) == -EINVAL
		);
		uassert((
			minix_inode_write(
				NANVIX_ROOT_DEV,
				&minix_fs.super,
				&inode,
				minix_fs.super.s_ninodes + 1)
			) == -EINVAL
		);

	uassert((
		minix_inode_free(
			&minix_fs.super,
			minix_fs.imap,
			ino)
		) == 0
	);
}

/**
 * @brief Sress Test: Block Alloc/Free
 */
static void minix_stress_block_alloc_free1(void)
{
	minix_block_t blocks[NANVIX_DISK_SIZE/MINIX_BLOCK_SIZE];

	for (unsigned i = 1; i < NANVIX_DISK_SIZE/MINIX_BLOCK_SIZE; i++)
	{
		uassert((
			blocks[i] = minix_block_alloc(
				&minix_fs.super,
				minix_fs.zmap
			)) != MINIX_BLOCK_NULL
		);

		uassert(
			minix_block_free_direct(
				&minix_fs.super,
				minix_fs.zmap,
				blocks[i]
			)  == 0
		);
	}
}

/**
 * @brief Sress Test: Block Alloc/Free
 */
static void minix_stress_block_alloc_free2(void)
{
	minix_block_t blocks[NANVIX_DISK_SIZE/MINIX_BLOCK_SIZE];

	for (unsigned i = 1; i < NANVIX_DISK_SIZE/MINIX_BLOCK_SIZE; i++)
	{
		uassert((
			blocks[i] = minix_block_alloc(
				&minix_fs.super,
				minix_fs.zmap
			)) != MINIX_BLOCK_NULL
		);
	}

	for (unsigned i = 1; i < NANVIX_DISK_SIZE/MINIX_BLOCK_SIZE; i++)
	{
		uassert(
			minix_block_free_direct(
				&minix_fs.super,
				minix_fs.zmap,
				blocks[i]
			)  == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Alloc/Free Sequential One Step
 */
static void minix_stress_inode_alloc_free1(void)
{
	minix_ino_t inos[NR_INODES];

	for (minix_ino_t i = 1; i < minix_fs.super.s_ninodes; i++)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&minix_fs.super,
				minix_fs.imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

		uassert((
			minix_inode_free(
				&minix_fs.super,
				minix_fs.imap,
				inos[i])
			) == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Alloc/Free Sequential Two Steps
 */
static void minix_stress_inode_alloc_free2(void)
{
	minix_ino_t inos[NR_INODES];

	for (minix_ino_t i = 1; i < minix_fs.super.s_ninodes; i++)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&minix_fs.super,
				minix_fs.imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);
	}

	for (minix_ino_t i = 1; i < minix_fs.super.s_ninodes; i++)
	{

		uassert((
			minix_inode_free(
				&minix_fs.super,
				minix_fs.imap,
				inos[i])
			) == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Read/Write Sequential One Step
 */
static void minix_stress_inode_read_write1(void)
{
	minix_ino_t inos[NR_INODES];
	struct d_inode inode;

	for (minix_ino_t i = 1; i < minix_fs.super.s_ninodes; i++)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&minix_fs.super,
				minix_fs.imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

			uassert((
				minix_inode_read(
					NANVIX_ROOT_DEV,
					&minix_fs.super,
					&inode,
					inos[i])
				) == 0
			);

			uassert((
				minix_inode_write(
					NANVIX_ROOT_DEV,
					&minix_fs.super,
					&inode,
					inos[i])
				) == 0
			);

		uassert((
			minix_inode_free(
				&minix_fs.super,
				minix_fs.imap,
				inos[i])
			) == 0
		);
	}
}

/**
 * @brief Stress Test: Inode Read/Write Sequential Two Steps
 */
static void minix_stress_inode_read_write2(void)
{
	minix_ino_t inos[NR_INODES];
	struct d_inode inode;

	for (minix_ino_t i = 1; i < minix_fs.super.s_ninodes; i++)
	{
		uassert((
			inos[i] = minix_inode_alloc(
				NANVIX_ROOT_DEV,
				&minix_fs.super,
				minix_fs.imap,
				0, 0, 0)
			) != MINIX_INODE_NULL
		);

	}

		for (minix_ino_t i = 1; i < minix_fs.super.s_ninodes; i++)
		{
			uassert((
				minix_inode_read(
					NANVIX_ROOT_DEV,
					&minix_fs.super,
					&inode,
					inos[i])
				) == 0
			);
		}

		for (minix_ino_t i = 1; i < minix_fs.super.s_ninodes; i++)
		{
			uassert((
				minix_inode_write(
					NANVIX_ROOT_DEV,
					&minix_fs.super,
					&inode,
					inos[i])
				) == 0
			);
		}

	for (minix_ino_t i = 1; i < minix_fs.super.s_ninodes; i++)
	{

		uassert((
			minix_inode_free(
				&minix_fs.super,
				minix_fs.imap,
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
	{ minix_api_block_alloc_free,     "[minix][api] block alloc/free         " },
	{ minix_api_inode_alloc_free,     "[minix][api] inode alloc/free         " },
	{ minix_api_inode_read_write,     "[minix][api] inode read/write         " },
	{ minix_fault_block_alloc_inval,  "[minix][fault] block alloc inval      " },
	{ minix_fault_block_free_inval,   "[minix][fault] block free inval       " },
	{ minix_fault_super_read_inval,   "[minix][fault] superblock read inval  " },
	{ minix_fault_super_write_inval,  "[minix][fault] superblock write inval " },
	{ minix_fault_inode_alloc_inval,  "[minix][fault] inode alloc inval      " },
	{ minix_fault_inode_free_inval,   "[minix][fault] inode free inval       " },
	{ minix_fault_inode_read_inval,   "[minix][fault] inode read inval       " },
	{ minix_fault_inode_write_inval,  "[minix][fault] inode write inval      " },
	{ minix_stress_block_alloc_free1, "[minix][stress] block alloc/free 1    " },
	{ minix_stress_block_alloc_free2, "[minix][stress] block alloc/free 2    " },
	{ minix_stress_inode_alloc_free1, "[minix][stress] inode alloc/free 1    " },
	{ minix_stress_inode_alloc_free2, "[minix][stress] inode alloc/free 2    " },
	{ minix_stress_inode_read_write1, "[minix][stress] inode read/write 1    " },
	{ minix_stress_inode_read_write2, "[minix][stress] inode read/write 2    " },
	{ NULL,                            NULL                                    },
};

/**
 * @brief Runs regression tests on RAM Disk.
 */
static void minix_test(void)
{
	for (int i = 0; minix_tests[i].func != NULL; i++)
	{
		minix_tests[i].func();

		uprintf("[nanvix][vfs] %s passed", minix_tests[i].name);
	}
}

/*============================================================================*
 * VFS Tests                                                                  *
 *============================================================================*/

/**
 * @brief Runs regression tests on VFS.
 */
void vfs_test(void)
{
	ramdisk_test();
	bcache_test();
	minix_test();
}
