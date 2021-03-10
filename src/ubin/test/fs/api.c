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

#include <nanvix/config.h>
#include <nanvix/fs.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/fcntl.h>
#include <posix/unistd.h>
#include <posix/stdlib.h>
#include "../test.h"

#ifdef __NANVIX_HAS_VFS_SERVER

/**
 * @brief File Offset for Tests
 */
#define TEST_FILE_OFFSET (8*NANVIX_FS_BLOCK_SIZE)

/**
 * @brief Buffer for Read/Write Tests
 */
static char data[NANVIX_FS_BLOCK_SIZE];

/*============================================================================*
 * Open/Close                                                                 *
 *============================================================================*/

/**
 * @brief API Test: Open/Close a File
 */
static void test_api_nanvix_vfs_open_close(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = nanvix_vfs_open(filename, O_RDONLY, 0)) >= 0);
	uassert(nanvix_vfs_close(fd) == 0);

	uassert((fd = nanvix_vfs_open(filename, O_WRONLY, 0)) >= 0);
	uassert(nanvix_vfs_close(fd) == 0);

	uassert((fd = nanvix_vfs_open(filename, O_RDWR, 0)) >= 0);
	uassert(nanvix_vfs_close(fd) == 0);
}

/*============================================================================*
 * Stat                                                                       *
 *============================================================================*/

/**
 * @brief API Test: Get File Stats
 */
static void test_api_nanvix_stat(void)
{
	struct nanvix_stat buffer;
	const char *filename = "disk";

	uassert((nanvix_vfs_stat(filename, &buffer)) >= 0);
}

/*============================================================================*
 * Seek                                                                       *
 *============================================================================*/

/**
 * @brief API Test: Seek Read/Write Pointer of a File
 */
static void test_api_nanvix_vfs_seek(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = nanvix_vfs_open(filename, O_RDWR, 0)) >= 0);

		uassert(nanvix_vfs_seek(fd, NANVIX_FS_BLOCK_SIZE, SEEK_CUR) >= 0);
		uassert(nanvix_vfs_seek(fd, 0, SEEK_END) >= 0);
		uassert(nanvix_vfs_seek(fd, NANVIX_FS_BLOCK_SIZE , SEEK_SET) >= 0);

	uassert(nanvix_vfs_close(fd) == 0);
}

/*============================================================================*
 * Read/Write                                                                 *
 *============================================================================*/

/**
 * @brief API Test: Read/Write from/to a File
 */
static void test_api_nanvix_vfs_read_write(void)
{
	int fd;
	const char *filename = "disk";
	const char *regfilename = "rdwr_file";

	uassert((fd = nanvix_vfs_open(filename, O_RDWR, 0)) >= 0);

		/* Write */
		umemset(data, 1, NANVIX_FS_BLOCK_SIZE);
		uassert(nanvix_vfs_seek(fd, TEST_FILE_OFFSET, SEEK_SET) >= 0);
		uassert(nanvix_vfs_write(fd, data, NANVIX_FS_BLOCK_SIZE) == NANVIX_FS_BLOCK_SIZE);

		/* Read. */
		umemset(data, 0, NANVIX_FS_BLOCK_SIZE);
		uassert(nanvix_vfs_seek(fd, TEST_FILE_OFFSET, SEEK_SET) >= 0);
		uassert(nanvix_vfs_read(fd, data, NANVIX_FS_BLOCK_SIZE) == NANVIX_FS_BLOCK_SIZE);

		/* Checksum. */
		for (size_t i = 0; i < sizeof(data); i++)
			uassert(data[i] == 1);

	uassert(nanvix_vfs_close(fd) == 0);

	/* Regular file */
	uassert((fd = nanvix_vfs_open(regfilename, (O_RDWR | O_CREAT),
					(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) >= 0);

		/* Write */
		umemset(data, 1, NANVIX_FS_BLOCK_SIZE);
		uassert(nanvix_vfs_seek(fd, TEST_FILE_OFFSET, SEEK_SET) >= 0);
		uassert(nanvix_vfs_write(fd, data, NANVIX_FS_BLOCK_SIZE) == NANVIX_FS_BLOCK_SIZE);

		/* Read. */
		umemset(data, 0, NANVIX_FS_BLOCK_SIZE);
		uassert(nanvix_vfs_seek(fd, TEST_FILE_OFFSET, SEEK_SET) >= 0);
		uassert(nanvix_vfs_read(fd, data, NANVIX_FS_BLOCK_SIZE) == NANVIX_FS_BLOCK_SIZE);

		/* Checksum. */
		for (size_t i = 0; i < sizeof(data); i++)
			uassert(data[i] == 1);

	uassert(nanvix_vfs_close(fd) == 0);
	uassert(nanvix_vfs_unlink(regfilename) == 0);
}

/**
 * @brief API Test: Create a file
 */
static void test_api_nanvix_vfs_creat(void)
{
	int fd;
	const char *filename = "new_file";

	uassert((fd = nanvix_vfs_open(filename, (O_CREAT | O_WRONLY), (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) >= 0);

	uassert(nanvix_vfs_close(fd) == 0);
}

/**
 * @brief API Test: Delete a file
 */
static void test_api_nanvix_vfs_unlink(void)
{
	char *filename = "new_file";
	uassert(nanvix_vfs_unlink(filename) == 0);
}

/*============================================================================*
 * API Tests                                                                  *
 *============================================================================*/

/**
 * @brief Virtual File System Tests
 */
struct test tests_vfs_api[] = {
	{ test_api_nanvix_vfs_open_close, "[vfs][api] open/close" },
	{ test_api_nanvix_vfs_seek,       "[vfs][api] seek      " },
	{ test_api_nanvix_vfs_read_write, "[vfs][api] read/write" },
	{ test_api_nanvix_stat,           "[vfs][api] stat      " },
	{ test_api_nanvix_vfs_creat,      "[vfs][api] creat     " },
	{ test_api_nanvix_vfs_unlink,     "[vfs][api] unlink    " },
	{ NULL,                            NULL                   },
};

#endif
