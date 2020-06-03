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
#include <nanvix/config.h>
#include <nanvix/dev.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/fcntl.h>
#include <posix/unistd.h>

/**
 * @brief Connection Used in Tests
 */
#define CONNECTION 0

/**
 * @brief Buffer for Read/Write Tests
 */
static char data[NANVIX_FS_BLOCK_SIZE];

/*============================================================================*
 * API Tests                                                                  *
 *============================================================================*/

/**
 * @brief API Test: Open/Close a File
 */
static void test_api_vfs_open_close(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = vfs_open(CONNECTION, filename, O_RDONLY, 0)) >= 0);
	uassert(vfs_close(CONNECTION, fd) == 0);

	uassert((fd = vfs_open(CONNECTION, filename, O_WRONLY, 0)) >= 0);
	uassert(vfs_close(CONNECTION, fd) == 0);

	uassert((fd = vfs_open(CONNECTION, filename, O_RDWR, 0)) >= 0);
	uassert(vfs_close(CONNECTION, fd) == 0);
}

/**
 * @brief API Test: Seek Read/Write Pointer of a File
 */
static void test_api_vfs_seek(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = vfs_open(CONNECTION, filename, O_RDWR, 0)) >= 0);

		uassert(vfs_seek(CONNECTION, fd, NANVIX_FS_BLOCK_SIZE, SEEK_CUR) >= 0);
		uassert(vfs_seek(CONNECTION, fd, 0, SEEK_END) >= 0);
		uassert(vfs_seek(CONNECTION, fd, NANVIX_FS_BLOCK_SIZE , SEEK_SET) >= 0);

	uassert(vfs_close(CONNECTION, fd) == 0);
}

/**
 * @brief API Test: Read/Write from/to a File
 */
static void test_api_vfs_read_write(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = vfs_open(CONNECTION, filename, O_RDWR, 0)) >= 0);

		uassert(vfs_seek(CONNECTION, fd, fs_root.super->data.s_first_data_block*NANVIX_FS_BLOCK_SIZE, SEEK_SET) >= 0);

		umemset(data, 1, NANVIX_FS_BLOCK_SIZE);
		uassert(vfs_write(CONNECTION, fd, data, NANVIX_FS_BLOCK_SIZE) == NANVIX_FS_BLOCK_SIZE);

		uassert(vfs_seek(CONNECTION, fd, fs_root.super->data.s_first_data_block*NANVIX_FS_BLOCK_SIZE, SEEK_SET) >= 0);

		umemset(data, 0, NANVIX_FS_BLOCK_SIZE);
		uassert(vfs_read(CONNECTION, fd, data, NANVIX_FS_BLOCK_SIZE) == NANVIX_FS_BLOCK_SIZE);

		/* Checksum. */
		for (size_t i = 0; i < sizeof(data); i++)
			uassert(data[i] == 1);

	uassert(vfs_close(CONNECTION, fd) == 0);
}

/*============================================================================*
 * Stress Tests                                                               *
 *============================================================================*/

/**
 * @brief Virtual File System Tests
 */
static struct
{
	void (*func)(void); /**< Test Function */
	const char *name;   /**< Test Name     */
} vfs_tests[] = {
	{ test_api_vfs_open_close, "[vfs][api] open/close" },
	{ test_api_vfs_seek,       "[vfs][api] seek      " },
	{ test_api_vfs_read_write, "[vfs][api] read/write" },
	{ NULL,                     NULL                   },
};

/**
 * @brief Runs regression tests on Virtual File System
 */
void test_vfs(void)
{
	for (int i = 0; vfs_tests[i].func != NULL; i++)
	{
		vfs_tests[i].func();

		uprintf("[nanvix][vfs]%s passed", vfs_tests[i].name);
	}
}
