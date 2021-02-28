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
 * @brief Buffer for Read/Write Tests
 */
static char data[NANVIX_FS_BLOCK_SIZE];

/*============================================================================*
 * Stat                                                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid stat
 */
static void test_fault_nanvix_vfs_stat_invalid(void)
{
	struct nanvix_stat buffer;

	uassert(nanvix_vfs_stat(NULL, &buffer) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Bad Stat
 */
static void test_fault_nanvix_vfs_stat_bad(void)
{
	const char *filename = "foobar";
	struct nanvix_stat buffer;

	uassert(nanvix_vfs_stat(filename, &buffer) == -ENOENT);
}

/*============================================================================*
 * Open                                                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Open
 */
static void test_fault_nanvix_vfs_open_invalid(void)
{
	const char *filename = "disk";
	const char *longname =
		"this file name is so long that should trigger an error";
	const char *emptyname = "";

	uassert(nanvix_vfs_open(filename, -1, 0) == -EINVAL);
	uassert(nanvix_vfs_open(emptyname, O_WRONLY, 0) == -EINVAL);
	uassert(nanvix_vfs_open(longname, O_WRONLY, 0) == -ENAMETOOLONG);
}

/**
 * @brief Fault Injection Test: Bad Open
 */
static void test_fault_nanvix_vfs_open_bad(void)
{
	const char *filename = "foobar";

	uassert(nanvix_vfs_open(filename, O_WRONLY, 0) == -ENOENT);
}

/**
 * @brief Fault Injection Test: Invalid Creat
 */
static void test_fault_nanvix_vfs_creat_invalid(void)
{
	const char *newfilename = "invalid_file";
	uassert(nanvix_vfs_open(newfilename, (O_CREAT | O_WRONLY), 0) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Bad Creat
 */
static void test_fault_nanvix_vfs_creat_bad(void)
{
	const char *newfilename = "bad_file";
	uassert(nanvix_vfs_open(newfilename, 0, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -ENOENT);
}

/*============================================================================*
 * Close                                                                      *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Close
 */
static void test_fault_nanvix_vfs_close_invalid(void)
{
	uassert(nanvix_vfs_close(-1) == -EINVAL);
	uassert(nanvix_vfs_close(NANVIX_NR_FILES) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Bad Close
 */
static void test_fault_nanvix_vfs_close_bad(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = nanvix_vfs_open(filename, O_RDONLY, 0)) >= 0);
	uassert(nanvix_vfs_close(fd + 1) == -EBADF);
	uassert(nanvix_vfs_close(fd) == 0);
}

/*============================================================================*
 * UNLINK                                                                     *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Unlink
 */
static void test_fault_nanvix_vfs_unlink_invalid(void)
{
	uassert(nanvix_vfs_unlink("/") == -EINVAL);
	uassert(nanvix_vfs_unlink("some_file") == -ENOENT);
}

/**
 * @brief Fault Injection Test: Bad Unlink
 */
static void test_fault_nanvix_vfs_unlink_bad(void)
{
	uassert(nanvix_vfs_unlink("") == -ENOENT);
	uassert(nanvix_vfs_unlink(NULL) == -EINVAL);
}

/*============================================================================*
 * Seek                                                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Seek
 */
static void test_fault_nanvix_vfs_seek_invalid(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = nanvix_vfs_open(filename, O_RDWR, 0)) >= 0);

		uassert(nanvix_vfs_seek(-1, NANVIX_FS_BLOCK_SIZE, SEEK_SET) == -EINVAL);
		uassert(nanvix_vfs_seek(NANVIX_NR_FILES, NANVIX_FS_BLOCK_SIZE, SEEK_SET) == -EINVAL);
		uassert(nanvix_vfs_seek(fd, -1, SEEK_SET) == -EINVAL);
		uassert(nanvix_vfs_seek(fd, NANVIX_FS_BLOCK_SIZE, -1) == -EINVAL);

	uassert(nanvix_vfs_close(fd) == 0);
}

/**
 * @brief Fault Injection Test: Bad Seek
 */
static void test_fault_nanvix_vfs_seek_bad(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = nanvix_vfs_open(filename, O_RDONLY, 0)) >= 0);
	uassert(nanvix_vfs_seek(fd + 1, NANVIX_FS_BLOCK_SIZE, SEEK_SET) == -EBADF);
	uassert(nanvix_vfs_close(fd) == 0);
}

/*============================================================================*
 * Read                                                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void test_fault_nanvix_vfs_read_invalid(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = nanvix_vfs_open(filename, O_RDWR, 0)) >= 0);

		uassert(nanvix_vfs_read(-1, data, NANVIX_FS_BLOCK_SIZE) == -EINVAL);
		uassert(nanvix_vfs_read(NANVIX_NR_FILES, data, NANVIX_FS_BLOCK_SIZE) == -EINVAL);
		uassert(nanvix_vfs_read(fd, NULL, NANVIX_FS_BLOCK_SIZE) == -EINVAL);
		uassert(nanvix_vfs_read(fd, data, 2*NANVIX_MAX_FILE_SIZE) == -EFBIG);

	uassert(nanvix_vfs_close(fd) == 0);
}

/**
 * @brief Fault Injection Test: Bad Read
 */
static void test_fault_nanvix_vfs_read_bad(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = nanvix_vfs_open(filename, O_RDONLY, 0)) >= 0);
	uassert(nanvix_vfs_read(fd + 1, data, NANVIX_FS_BLOCK_SIZE) == -EBADF);
	uassert(nanvix_vfs_close(fd) == 0);
}

/*============================================================================*
 * Write                                                                      *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Write
 */
static void test_fault_nanvix_vfs_write_invalid(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = nanvix_vfs_open(filename, O_RDWR, 0)) >= 0);

		uassert(nanvix_vfs_write(-1, data, NANVIX_FS_BLOCK_SIZE) == -EINVAL);
		uassert(nanvix_vfs_write(NANVIX_NR_FILES, data, NANVIX_FS_BLOCK_SIZE) == -EINVAL);
		uassert(nanvix_vfs_write(fd, NULL, NANVIX_FS_BLOCK_SIZE) == -EINVAL);
		uassert(nanvix_vfs_write(fd, data, 2*NANVIX_MAX_FILE_SIZE) == -EFBIG);

	uassert(nanvix_vfs_close(fd) == 0);
}

/**
 * @brief Fault Injection Test: Bad Write
 */
static void test_fault_nanvix_vfs_write_bad(void)
{
	int fd;
	const char *filename = "disk";

	uassert((fd = nanvix_vfs_open(filename, O_RDONLY, 0)) >= 0);
	uassert(nanvix_vfs_write(fd + 1, data, NANVIX_FS_BLOCK_SIZE) == -EBADF);
	uassert(nanvix_vfs_close(fd) == 0);
}

/*============================================================================*
 * Fault Injection Tests                                                      *
 *============================================================================*/

/**
 * @brief Virtual File System Tests
 */
struct test tests_vfs_fault[] = {
	{ test_fault_nanvix_vfs_open_invalid,  "[vfs][fault] invalid open  " },
	{ test_fault_nanvix_vfs_open_bad,      "[vfs][fault] bad open      " },
	{ test_fault_nanvix_vfs_creat_invalid, "[vfs][fault] invalid creat " },
	{ test_fault_nanvix_vfs_creat_bad,     "[vfs][fault] bad creat     " },
	{ test_fault_nanvix_vfs_close_invalid, "[vfs][fault] invalid close " },
	{ test_fault_nanvix_vfs_close_bad,     "[vfs][fault] bad close     " },
	{ test_fault_nanvix_vfs_seek_invalid,  "[vfs][fault] invalid seek  " },
	{ test_fault_nanvix_vfs_seek_bad,      "[vfs][fault] bad seek      " },
	{ test_fault_nanvix_vfs_read_invalid,  "[vfs][fault] invalid read  " },
	{ test_fault_nanvix_vfs_read_bad,      "[vfs][fault] bad read      " },
	{ test_fault_nanvix_vfs_write_invalid, "[vfs][fault] invalid write " },
	{ test_fault_nanvix_vfs_write_bad,     "[vfs][fault] bad write     " },
	{ test_fault_nanvix_vfs_stat_invalid,  "[vfs][fault] invalid stat  " },
	{ test_fault_nanvix_vfs_stat_bad,      "[vfs][fault] bad stat      " },
	{ test_fault_nanvix_vfs_unlink_invalid,"[vfs][fault] invalid unlink" },
	{ test_fault_nanvix_vfs_unlink_bad,    "[vfs][fault] bad unlink    " },
	{ NULL,                                 NULL                         },
};

#endif
