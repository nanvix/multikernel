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
#include <nanvix/sys/noc.h>
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
 * @brief Number of Iterations
 */
#define TEST_NITERATIONS 128

/**
 * @brief File Offset for Tests
 */
#define TEST_FILE_OFFSET (8*NANVIX_FS_BLOCK_SIZE)

/**
 * @brief Buffer for Read/Write Tests
 */
static char data[NANVIX_FS_BLOCK_SIZE];

/*============================================================================*
 * Stat                                                                       *
 *============================================================================*/

/**
 * @brief Stress Test: Get File Stats
 */
static void test_stress_nanvix_vfs_stat(void)
{
	int fd;
	struct nanvix_stat buffer;
	const char *filename = "disk";

	for (int i = 0; i < TEST_NITERATIONS; i++)
	{
		uassert((fd = nanvix_vfs_stat(filename, &buffer)) >= 0);
	}
}

/*============================================================================*
 * Open/Close                                                                 *
 *============================================================================*/

/**
 * @brief Stress Test: Open/Close a File
 */
static void test_stress_nanvix_vfs_open_close(void)
{
	int fd;
	const char *filename = "disk";

	for (int i = 0; i < NANVIX_OPEN_MAX; i++)
	{
		uassert((fd = nanvix_vfs_open(filename, O_RDONLY, 0)) >= 0);
		uassert(nanvix_vfs_close(fd) == 0);
	}

	for (int i = 0; i < NANVIX_OPEN_MAX; i++)
	{
		uassert((fd = nanvix_vfs_open(filename, O_WRONLY, 0)) >= 0);
		uassert(nanvix_vfs_close(fd) == 0);
	}

	for (int i = 0; i < NANVIX_OPEN_MAX; i++)
	{
		uassert((fd = nanvix_vfs_open(filename, O_RDWR, 0)) >= 0);
		uassert(nanvix_vfs_close(fd) == 0);
	}
}

/*============================================================================*
 * Creat/Unlink                                                               *
 *============================================================================*/

/**
 * @brief Stress Test: Creat/Unlink a File
 */
static void test_stress_nanvix_vfs_creat_unlink(void)
{
	char *filename = "stress_file";

	for (int i = 0; i < NANVIX_OPEN_MAX; i++)
	{
		uassert(nanvix_vfs_open(filename, (O_WRONLY | O_CREAT), (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) >= 0);
		uassert(nanvix_vfs_unlink(filename) == 0);
	}

	for (int i = 0; i < NANVIX_OPEN_MAX; i++)
	{
		uassert(nanvix_vfs_open(filename, (O_RDWR | O_CREAT), (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) >= 0);
		uassert(nanvix_vfs_unlink(filename) == 0);
	}
}

/*============================================================================*
 * Seek                                                                       *
 *============================================================================*/

/**
 * @brief Stress Test: Seek Read/Write Pointer of a File
 */
static void test_stress_nanvix_vfs_seek(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = nanvix_vfs_open(filename, O_RDWR, 0)) >= 0);

	for (int i = 0; i < TEST_NITERATIONS; i++)
	{
		uassert(nanvix_vfs_seek(fd, NANVIX_FS_BLOCK_SIZE, SEEK_CUR) >= 0);
		uassert(nanvix_vfs_seek(fd, 0, SEEK_END) >= 0);
		uassert(nanvix_vfs_seek(fd, NANVIX_FS_BLOCK_SIZE , SEEK_SET) >= 0);
	}

	uassert(nanvix_vfs_close(fd) == 0);
}

/*============================================================================*
 * Read/Write                                                                 *
 *============================================================================*/

/**
 * @brief Stress Test: Read/Write from/to a File
 */
static void test_stress_nanvix_vfs_read_write(void)
{
	int fd;
	int nfd;
	const char *filename = "disk";
	const char *newfilename = "new_file";

	uassert((fd = nanvix_vfs_open(filename, O_RDWR, 0)) >= 0);
	uassert((nfd = nanvix_vfs_open(newfilename, (O_CREAT | O_RDWR), (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) >= 0);

	for (int i = 0; i < TEST_NITERATIONS; i++)
	{
		/* Write */
		umemset(data, 1, NANVIX_FS_BLOCK_SIZE);
		uassert(nanvix_vfs_seek(fd, TEST_FILE_OFFSET, SEEK_SET) >= 0);
		uassert(nanvix_vfs_write(fd, data, NANVIX_FS_BLOCK_SIZE) == NANVIX_FS_BLOCK_SIZE);

		/* Read. */
		umemset(data, 0, NANVIX_FS_BLOCK_SIZE);
		uassert(nanvix_vfs_seek(fd, TEST_FILE_OFFSET, SEEK_SET) >= 0);
		uassert(nanvix_vfs_read(fd, data, NANVIX_FS_BLOCK_SIZE) == NANVIX_FS_BLOCK_SIZE);

		/* Checksum. */
		for (size_t j = 0; j < sizeof(data); j++)
			uassert(data[j] == 1);

		/* Write regular file */
		umemset(data, 1, NANVIX_FS_BLOCK_SIZE);
		uassert(nanvix_vfs_seek(nfd, TEST_FILE_OFFSET, SEEK_SET) >= 0);
		uassert(nanvix_vfs_write(nfd, data, NANVIX_FS_BLOCK_SIZE) == NANVIX_FS_BLOCK_SIZE);

		/* Read regular file */
		umemset(data, 0, NANVIX_FS_BLOCK_SIZE);
		uassert(nanvix_vfs_seek(nfd, TEST_FILE_OFFSET, SEEK_SET) >= 0);
		uassert(nanvix_vfs_read(nfd, data, NANVIX_FS_BLOCK_SIZE) == NANVIX_FS_BLOCK_SIZE);

		/* Checksum. */
		for (size_t j = 0; j < sizeof(data); j++)
			uassert(data[j] == 1);
	}

	uassert(nanvix_vfs_close(fd) == 0);
	uassert(nanvix_vfs_unlink("new_file") == 0);
}

/*============================================================================*
 * Stress Tests                                                               *
 *============================================================================*/

/**
 * @brief Virtual File System Tests
 */
struct test tests_vfs_stress[] = {
	{ test_stress_nanvix_vfs_open_close,   "[vfs][stress] open/close    " },
	{ test_stress_nanvix_vfs_seek,         "[vfs][stress] seek          " },
	{ test_stress_nanvix_vfs_read_write,   "[vfs][stress] read/write    " },
	{ test_stress_nanvix_vfs_stat,         "[vfs][stress] stat          " },
	{ test_stress_nanvix_vfs_creat_unlink, "[vfs][stress] creat/unlink  " },
	{ NULL,                                NULL                           },
};

#endif
