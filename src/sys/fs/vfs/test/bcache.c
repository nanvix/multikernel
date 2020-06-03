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

/*============================================================================*
 * Block Cache Tests                                                          *
 *============================================================================*/

/**
 * @brief API Test: Block Read/Release
 */
static void test_api_bcache_bread_brelse(void)
{
	struct buffer *buf;

	uassert((buf = bread(0, 0)) != NULL);
	uassert(brelse(buf) == 0);
}

/**
 * @brief API Test: Block Read/Write
 */
static void test_api_bcache_bread_bwrite(void)
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
static void test_fault_bcache_bread_inval(void)
{
	uassert(bread(-1, 0) == NULL);
	uassert(bread(0, -1) == NULL);
	uassert(bread(0, NANVIX_DISK_SIZE/NANVIX_FS_BLOCK_SIZE) == NULL);
}

/**
 * @brief Fault Injection Test: Invalid Release
 */
static void test_fault_bcache_brelse_inval(void)
{
	uassert(brelse(NULL) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Invalid Write
 */
static void test_fault_bcache_bwrite_inval(void)
{
	uassert(bwrite(NULL) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Bad Release
 */
static void test_fault_bcache_brelse_bad(void)
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
static void test_fault_bcache_bwrite_bad(void)
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
static void test_stress_bcache_bread_brelse(void)
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
static void test_stress_bcache_bread_bwrite(void)
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
	{ test_api_bcache_bread_brelse,    "[bcache][api] bread/brelse     " },
	{ test_api_bcache_bread_bwrite,    "[bcache][api] bread/bwrite     " },
	{ test_fault_bcache_bread_inval,   "[bcache][fault] invalid bread  " },
	{ test_fault_bcache_brelse_inval,  "[bcache][fault] invalid brelse " },
	{ test_fault_bcache_bwrite_inval,  "[bcache][fault] invalid bwrite " },
	{ test_fault_bcache_brelse_bad,    "[bcache][fault] bad brelse     " },
	{ test_fault_bcache_bwrite_bad,    "[bcache][fault] bad bwrite     " },
	{ test_stress_bcache_bread_brelse, "[bcache][stress] bread/brelse  " },
	{ test_stress_bcache_bread_bwrite, "[bcache][stress] bread/bwrite  " },
	{ NULL,                             NULL                             },
};

/**
 * @brief Runs regression tests on Block Cache
 */
void test_bcache(void)
{
	for (int i = 0; bcache_tests[i].func != NULL; i++)
	{
		bcache_tests[i].func();

		uprintf("[nanvix][vfs]%s passed", bcache_tests[i].name);
	}
}
