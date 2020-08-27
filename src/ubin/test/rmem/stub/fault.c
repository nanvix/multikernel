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

#define __NEED_MM_RMEM_STUB

#include <nanvix/runtime/mm.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include "../../test.h"

/**
 * @brief Run bad read tests?
 */
#define __TEST_BAD_READ 1

/**
 * @brief Run bad write tests?
 */
#define __TEST_BAD_WRITE 1

/**
 * @brief Dummy buffer.
 */
static char buffer[RMEM_BLOCK_SIZE];

/*============================================================================*
 * Fault Injection Test: Invalid Free                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Free
 */
static void test_rmem_stub_invalid_free(void)
{
	TEST_ASSERT(nanvix_rmem_free(RMEM_NULL) == -EINVAL);
	TEST_ASSERT(nanvix_rmem_free(RMEM_NUM_BLOCKS) == -EINVAL);
}

/*============================================================================*
 * Fault Injection Test: Bad Free                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Free
 */
static void test_rmem_stub_bad_free(void)
{
	TEST_ASSERT(nanvix_rmem_free(1) == -EFAULT);
	TEST_ASSERT(nanvix_rmem_free(RMEM_NUM_BLOCKS - 1) == -EFAULT);
}

/*============================================================================*
 * Fault Injection Test: Invalid Write                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Write
 */
static void test_rmem_stub_invalid_write(void)
{
	rpage_t blknum;

	umemset(buffer, 1, RMEM_BLOCK_SIZE);

	/* Invalid block number. */
	TEST_ASSERT(nanvix_rmem_write(RMEM_NULL, buffer) == 0);
	TEST_ASSERT(nanvix_rmem_write(RMEM_NUM_BLOCKS, buffer) == 0);
	TEST_ASSERT(nanvix_rmem_write(RMEM_NUM_BLOCKS + 1, buffer) == 0);

	/* Invalid buffer. */
	TEST_ASSERT((blknum = nanvix_rmem_alloc()) != RMEM_NULL);
	TEST_ASSERT(nanvix_rmem_write(blknum, NULL) == 0);
	TEST_ASSERT(nanvix_rmem_free(blknum) == 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Write                                            *
 *============================================================================*/

#if __TEST_BAD_WRITE

/**
 * @brief Fault Injection Test: Bad Write
 */
static void test_rmem_stub_bad_write(void)
{
	umemset(buffer, 1, RMEM_BLOCK_SIZE);

	TEST_ASSERT(nanvix_rmem_write(1, buffer) == 0);
	TEST_ASSERT(nanvix_rmem_write(RMEM_NUM_BLOCKS - 1, buffer) == 0);
}

#endif

/*============================================================================*
 * Fault Injection Test: Invalid Read                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void test_rmem_stub_invalid_read(void)
{
	rpage_t blknum;

	umemset(buffer, 1, RMEM_BLOCK_SIZE);

	/* Invalid block number. */
	TEST_ASSERT(nanvix_rmem_read(RMEM_NULL, buffer) == 0);
	TEST_ASSERT(nanvix_rmem_read(RMEM_NUM_BLOCKS, buffer) == 0);
	TEST_ASSERT(nanvix_rmem_read(RMEM_NUM_BLOCKS + 1, buffer) == 0);

	/* Invalid buffer. */
	TEST_ASSERT((blknum = nanvix_rmem_alloc()) != RMEM_NULL);
	TEST_ASSERT(nanvix_rmem_read(blknum, NULL) == 0);
	TEST_ASSERT(nanvix_rmem_free(blknum) == 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Read                                             *
 *============================================================================*/

#if __TEST_BAD_READ

/**
 * @brief Fault Injection Test: Bad Read
 */
static void test_rmem_stub_bad_read(void)
{
	umemset(buffer, 1, RMEM_BLOCK_SIZE);

	TEST_ASSERT(nanvix_rmem_read(1, buffer) == 0);
	TEST_ASSERT(nanvix_rmem_read(RMEM_NUM_BLOCKS - 1, buffer) == 0);
}

#endif

/*============================================================================*
 * Fault Injection Test: Invalid Stats                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Stats
 */
static void test_rmem_stub_invalid_stats(void)
{
	TEST_ASSERT(nanvix_rmem_stats(NULL) == -EINVAL);
}

/*============================================================================*
 * Test Driver Table                                                          *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_stub_fault[] = {
	{ test_rmem_stub_invalid_free,  "invalid free " },
	{ test_rmem_stub_bad_free,      "bad free     " },
	{ test_rmem_stub_invalid_write, "invalid write" },
#if __TEST_BAD_WRITE
	{ test_rmem_stub_bad_write,     "bad write    " },
#endif
	{ test_rmem_stub_invalid_read,  "invalid read " },
#if __TEST_BAD_READ
	{ test_rmem_stub_bad_read,      "bad read     " },
#endif
	{ test_rmem_stub_invalid_stats, "invalid stats" },
	{ NULL,                          NULL           },
};
