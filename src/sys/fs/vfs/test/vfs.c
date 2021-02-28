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
#include <posix/stdlib.h>

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
 * @brief API Test: Get File Stats
 */
static void test_api_vfs_stat(void)
{
	struct nanvix_stat *restrict buffer = nanvix_malloc(sizeof(struct nanvix_stat *restrict));
	const char *filename = "disk";

	uassert(fs_stat(filename, buffer) >= 0);
	uassert(buffer != NULL);

	/* TODO: test time related fields */
	/* test if data is actually there */
	uassert(buffer->st_dev > 0);
	uassert(buffer->st_ino > 0);
	uassert(buffer->st_mode > 0);
	uassert(buffer->st_nlink > 0);
	uassert(buffer->st_uid > 0);
	uassert(buffer->st_gid > 0);
	uassert(buffer->st_rdev > 0);
	uassert(buffer->st_size > 0);
	uassert(buffer->st_blksize > 0);
	uassert(buffer->st_blocks > 0);
}

/**
 * @brief API Test: Get Stats of Inexisting File
 */
static void test_api_vfs_stat_file_not_exists(void)
{
	struct nanvix_stat *restrict buffer = nanvix_malloc(sizeof(struct nanvix_stat *restrict));
	const char *filename = "inexistent";
	
	uassert(fs_stat(filename, buffer) == -ENOENT);
}

/**
 * @brief API Test: Get Stats of Invalid File
 */
static void test_api_vfs_stat_file_invalid(void)
{
	struct nanvix_stat *restrict buffer = nanvix_malloc(sizeof(struct nanvix_stat *restrict));
	const char *filename = "";
	
	uassert(fs_stat(filename, buffer) == -EINVAL);
}

/**
 * @brief API Test: Get Stats Using Invalid Buffer
 */
static void test_api_vfs_stat_buffer_invalid(void)
{
	struct nanvix_stat *restrict buffer = NULL;
	const char *filename = "disk";
	
	uassert(fs_stat(filename, buffer) == -1);
}

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
 * @brief API Test: Create/Unlink a File
 */
static void test_api_vfs_creat_unlink(void)
{
	int fd;
	const char *filename = "new_file";

	uassert((fd = vfs_open(CONNECTION, filename, (O_RDONLY | O_CREAT), (S_IRWXU | S_RGRP | S_ROTH))) >= 0);
	uassert(vfs_unlink(CONNECTION, filename) == 0);
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
	{ test_api_vfs_open_close,           "[vfs][api] open/close          " },
	{ test_api_vfs_seek,                 "[vfs][api] seek                " },
	{ test_api_vfs_read_write,           "[vfs][api] read/write          " },
	{ test_api_vfs_stat,                 "[vfs][api] stat                " },
	{ test_api_vfs_creat_unlink,         "[vfs][api] create/unlink       " },
	{ test_api_vfs_stat_file_not_exists, "[vfs][api] stat no file        " },
	{ test_api_vfs_stat_file_invalid,    "[vfs][api] stat invalid file   " },
	{ test_api_vfs_stat_buffer_invalid,  "[vfs][api] stat invalid buffer " },
	{ NULL,                               NULL                             },
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
