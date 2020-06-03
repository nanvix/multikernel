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
#include "../../test.h"

/**
 * @brief Verbose tests?
 */
#define __VERBOSE_TESTS 0

/**
 * @brief Run alloc/free all test?
 */
#define __TEST_ALLOC_FREE_ALL 0

/**
 * @brief run read/write all test?
 */
#define __TEST_READ_WRITE_ALL 0

/**
 * @brief Number of blocks to allocate.
 */
#define NUM_BLOCKS 256

/**
 * @brief Dummy buffer 1.
 */
static char buffer1[RMEM_BLOCK_SIZE];

/**
 * @brief Dummy buffer 2.
 */
static char buffer2[RMEM_BLOCK_SIZE];

/**
 * @brief Dummy buffer 3.
 */
static unsigned buffer3[RMEM_BLOCK_SIZE/sizeof(unsigned)];

/*============================================================================*
 * Stress Test: Alloc/Free Sequential                                         *
 *============================================================================*/

/**
 * @brief Stress Test: Alloc/Free Sequential
 */
static void test_rmem_stub_alloc_free_sequential(void)
{
	rpage_t blks[NUM_BLOCKS];

	for (unsigned long i = 0; i < NUM_BLOCKS; i++)
	{
		TEST_ASSERT((blks[i] = nanvix_rmem_alloc()) != RMEM_NULL);
		#if (__VERBOSE_TESTS)
			uprintf("rmem_alloc() blknum=%d", blks[i]);
		#endif
	}
	for (unsigned long i = 0; i < NUM_BLOCKS; i++)
	{
		#if (__VERBOSE_TESTS)
			uprintf("rmem_free()  blknum=%d", blks[i]);
		#endif
		TEST_ASSERT(nanvix_rmem_free(blks[i]) == 0);
	}
}

/*============================================================================*
 * Stress Test: Alloc/Free Interleaved                                        *
 *============================================================================*/

/**
 * @brief Stress Test: Alloc/Free
 */
static void test_rmem_stub_alloc_free_interleaved(void)
{
	rpage_t blks[NUM_BLOCKS];

	for (unsigned long i = 0; i < NUM_BLOCKS; i++)
	{
		TEST_ASSERT((blks[i] = nanvix_rmem_alloc()) != RMEM_NULL);
		#if (__VERBOSE_TESTS)
			uprintf("rmem_alloc() blknum=%d", blks[i]);
		#endif
	}
	for (unsigned long i = 0; i < NUM_BLOCKS; i += 2)
	{
		TEST_ASSERT(nanvix_rmem_free(blks[i]) == 0);
		#if (__VERBOSE_TESTS)
			uprintf("rmem_free()  blknum=%d", blks[i]);
		#endif
	}
	for (unsigned long i = 1; i < NUM_BLOCKS; i += 2)
	{
		TEST_ASSERT(nanvix_rmem_free(blks[i]) == 0);
		#if (__VERBOSE_TESTS)
			uprintf("rmem_free()  blknum=%d", blks[i]);
		#endif
	}
}

/*============================================================================*
 * Stress Test: Alloc/Free All                                                *
 *============================================================================*/

#if __TEST_ALLOC_FREE_ALL

/**
 * @brief Stress Test: Alloc/Free All
 */
static void test_rmem_stub_alloc_free_all(void)
{

	for (unsigned long i = 1; i < RMEM_NUM_BLOCKS; i++)
	{
		rpage_t blknum;

		TEST_ASSERT((blknum = nanvix_rmem_alloc()) != RMEM_NULL);
		#if (__VERBOSE_TESTS)
			uprintf("rmem_alloc() blknum=%d", blknum);
		#endif
	}
	for (unsigned long i = 1; i < RMEM_NUM_BLOCKS; i++)
	{
		rpage_t blknum = i;

		TEST_ASSERT(nanvix_rmem_free(blknum) == 0);
		#if (__VERBOSE_TESTS)
			uprintf("rmem_free()  blknum=%d", blknum);
		#endif
	}
}

/*============================================================================*
 * Stress Test: Alloc Overflow                                                *
 *============================================================================*/

/**
 * @brief Stress Test: Alloc Overflow
 */
static void test_rmem_stub_alloc_overflow(void)
{
	static rpage_t blks[RMEM_NUM_BLOCKS];

	/* Allocate all blocks. */
	for (unsigned long i = 1; i < RMEM_NUM_BLOCKS; i++)
		TEST_ASSERT((blks[i] = nanvix_rmem_alloc()) != RMEM_NULL);

	/* Fail. */
	TEST_ASSERT(nanvix_rmem_alloc() == RMEM_NULL);

	/* Free all blocks. */
	for (unsigned long i = 1; i < RMEM_NUM_BLOCKS; i++)
		TEST_ASSERT(nanvix_rmem_free(blks[i]) == 0);
}

#endif

/*============================================================================*
 * Stress Test: Read/Write Sequential                                         *
 *============================================================================*/

/**
 * @brief Stress Test: Read/Write Sequential
 */
static void test_rmem_stub_read_write_sequential(void)
{
	rpage_t blks[NUM_BLOCKS];

	/* Allocate many blocks.*/
	for (unsigned long i = 0; i < NUM_BLOCKS; i++)
		TEST_ASSERT((blks[i] = nanvix_rmem_alloc()) != RMEM_NULL);

	/* Read and write. */
	for (unsigned long i = 0; i < NUM_BLOCKS; i++)
	{
		umemset(buffer1, i + 1, RMEM_BLOCK_SIZE);
		umemset(buffer2, 0, RMEM_BLOCK_SIZE);

		#if (__VERBOSE_TESTS)
			uprintf("rmem_write() blknum=%d", blks[i]);
		#endif
		TEST_ASSERT(nanvix_rmem_write(blks[i], buffer1) == RMEM_BLOCK_SIZE);

		#if (__VERBOSE_TESTS)
			uprintf("rmem_read()  blknum=%d", blks[i]);
		#endif
		TEST_ASSERT(nanvix_rmem_read(blks[i], buffer2) == RMEM_BLOCK_SIZE);
		TEST_ASSERT(umemcmp(buffer1, buffer2, RMEM_BLOCK_SIZE) == 0);

	}

	/* Free all blocks. */
	for (unsigned long i = 0; i < NUM_BLOCKS; i++)
		TEST_ASSERT(nanvix_rmem_free(blks[i]) == 0);
}

/*============================================================================*
 * Stress Test: Read/Write Interleaved                                        *
 *============================================================================*/

/**
 * @brief Stress Test: Read/Write Interleaved
 */
static void test_rmem_stub_read_write_interleaved(void)
{
	rpage_t blks[NUM_BLOCKS];

	/* Allocate many blocks.*/
	for (unsigned long i = 0; i < NUM_BLOCKS; i++)
		TEST_ASSERT((blks[i] = nanvix_rmem_alloc()) != RMEM_NULL);

	/* Read and write. */
	for (int j = 0; j < 2; j++)
	{
		for (unsigned long i = j; i < NUM_BLOCKS; i +=2)
		{
			umemset(buffer1, i + 1, RMEM_BLOCK_SIZE);

			#if (__VERBOSE_TESTS)
				uprintf("rmem_write() blknum=%d", blks[i]);
			#endif
			TEST_ASSERT(nanvix_rmem_write(blks[i], buffer1) == RMEM_BLOCK_SIZE);
		}
	}
	for (int j = 0; j < 2; j++)
	{
		for (unsigned long i = j; i < NUM_BLOCKS; i +=2)
		{
			umemset(buffer1, i + 1, RMEM_BLOCK_SIZE);
			umemset(buffer2, 0, RMEM_BLOCK_SIZE);

			#if (__VERBOSE_TESTS)
				uprintf("rmem_read()  blknum=%d", blks[i]);
			#endif
			TEST_ASSERT(nanvix_rmem_read(blks[i], buffer2) == RMEM_BLOCK_SIZE);
			TEST_ASSERT(umemcmp(buffer1, buffer2, RMEM_BLOCK_SIZE) == 0);
		}
	}

	/* Free all blocks. */
	for (unsigned long i = 0; i < NUM_BLOCKS; i++)
		TEST_ASSERT(nanvix_rmem_free(blks[i]) == 0);
}

/*============================================================================*
 * Stress Test: Read/Write All                                                *
 *============================================================================*/

#if __TEST_READ_WRITE_ALL

/**
 * @brief Stress Test: Read/Write All
 */
static void test_rmem_stub_read_write_all(void)
{

	for (unsigned long i = 1; i < RMEM_NUM_BLOCKS; i++)
	{
		rpage_t blknum;

		umemset(buffer1, i + 1, RMEM_BLOCK_SIZE);

		TEST_ASSERT((blknum = nanvix_rmem_alloc()) != RMEM_NULL);
		TEST_ASSERT(nanvix_rmem_write(blknum, buffer1) == RMEM_BLOCK_SIZE);

		#if (__VERBOSE_TESTS)
			uprintf("rmem_write() blknum=%d", blknum);
		#endif
	}
	for (unsigned long i = 1; i < RMEM_NUM_BLOCKS; i++)
	{
		rpage_t blknum = i;

		umemset(buffer1, i + 1, RMEM_BLOCK_SIZE);
		umemset(buffer2, 0, RMEM_BLOCK_SIZE);

		#if (__VERBOSE_TESTS)
			uprintf("rmem_read()  blknum=%d", blknum);
		#endif

		TEST_ASSERT(nanvix_rmem_read(blknum, buffer2) == RMEM_BLOCK_SIZE);
		TEST_ASSERT(umemcmp(buffer1, buffer2, RMEM_BLOCK_SIZE) == 0);
		TEST_ASSERT(nanvix_rmem_free(blknum) == 0);
	}
}

#endif

/*============================================================================*
 * Stress Test: Consistency                                                   *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency
 */
static void test_rmem_stub_consistency_raw(void)
{
	rpage_t numbers;

	for (unsigned i = 1; i <= NUM_BLOCKS; i++)
	{
		TEST_ASSERT((numbers = nanvix_rmem_alloc()) != RMEM_NULL);

		umemset(buffer1, i + 1, RMEM_BLOCK_SIZE);
		umemset(buffer2, i + 1, RMEM_BLOCK_SIZE);

		TEST_ASSERT(nanvix_rmem_write(numbers, buffer1) == RMEM_BLOCK_SIZE);

		umemset(buffer1, 0, RMEM_BLOCK_SIZE);

		TEST_ASSERT(nanvix_rmem_read(numbers, buffer1) == RMEM_BLOCK_SIZE);

		TEST_ASSERT(umemcmp(buffer1, buffer2, RMEM_BLOCK_SIZE) == 0);

		TEST_ASSERT(nanvix_rmem_free(numbers) == 0);
	}
}

/*============================================================================*
 * Stress Test: Consistency                                                   *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency
 */
static void test_rmem_stub_consistency(void)
{
	rpage_t numbers;

	for (unsigned i = 1; i <= NUM_BLOCKS; i++)
	{
		TEST_ASSERT((numbers = nanvix_rmem_alloc()) != RMEM_NULL);

		for (unsigned j = 0; j < RMEM_BLOCK_SIZE/sizeof(unsigned); j++)
			buffer3[j] = (i - 1)*RMEM_NUM_BLOCKS + j;

		TEST_ASSERT(nanvix_rmem_write(numbers, buffer3) == RMEM_BLOCK_SIZE);
		TEST_ASSERT(nanvix_rmem_read(numbers, buffer3) == RMEM_BLOCK_SIZE);

		for (unsigned j = 0; j < RMEM_BLOCK_SIZE/sizeof(unsigned); j++)
			TEST_ASSERT(buffer3[j] == (i - 1)*RMEM_NUM_BLOCKS + j);

		TEST_ASSERT(nanvix_rmem_free(numbers) == 0);
	}
}

/*============================================================================*
 * Stress Test: Consistency 2-Step                                            *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency 2-Step
 */
static void test_rmem_stub_consistency2(void)
{
	rpage_t numbers[NUM_BLOCKS];

	for (unsigned i = 1; i <= NUM_BLOCKS; i++)
	{
		TEST_ASSERT((numbers[i - 1] = nanvix_rmem_alloc()) != RMEM_NULL);

		for (unsigned j = 0; j < RMEM_BLOCK_SIZE/sizeof(unsigned); j++)
			buffer3[j] = (i - 1)*RMEM_NUM_BLOCKS + j;

		TEST_ASSERT(nanvix_rmem_write(numbers[i - 1], buffer3) == RMEM_BLOCK_SIZE);
	}

	for (unsigned i = 1; i <= NUM_BLOCKS; i++)
	{
		TEST_ASSERT(nanvix_rmem_read(numbers[i - 1], buffer3) == RMEM_BLOCK_SIZE);

		for (unsigned j = 0; j < RMEM_BLOCK_SIZE/sizeof(unsigned); j++)
			TEST_ASSERT(buffer3[j] == (i - 1)*RMEM_NUM_BLOCKS + j);

		TEST_ASSERT(nanvix_rmem_free(numbers[i - 1]) == 0);
	}
}

/*============================================================================*
 * Test Driver Table                                                          *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_stub_stress[] = {
	{ test_rmem_stub_alloc_free_sequential,  "alloc/free sequential " },
	{ test_rmem_stub_alloc_free_interleaved, "alloc/free interleaved" },
	{ test_rmem_stub_consistency_raw,        "consistency raw       " },
	{ test_rmem_stub_consistency,            "consistency           " },
	{ test_rmem_stub_consistency2,           "consistency 2-step    " },
#if __TEST_ALLOC_FREE_ALL
	{ test_rmem_stub_alloc_free_all,         "alloc/free all        " },
	{ test_rmem_stub_alloc_overflow,         "alloc overflow        " },
#endif
	{ test_rmem_stub_read_write_sequential,  "read/write sequential " },
	{ test_rmem_stub_read_write_interleaved, "read/write interleaved" },
#if __TEST_READ_WRITE_ALL
	{ test_rmem_stub_read_write_all,         "read/write all        " },
#endif
	{ NULL,                                   NULL                    },
};
