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

#define __NEED_MM_RMEM_CACHE

#include <nanvix/runtime/mm.h>
#include <nanvix/ulib.h>
#include "../../test.h"

/**
 * @brief Number of blocks to allocate.
 */
#define NUM_BLOCKS 256

/*============================================================================*
 * Stress Test: Consistency                                                   *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency
 */
static void test_rmem_rcache_consistency(void)
{
	rpage_t numbers;
	unsigned *cached_data;

	for (unsigned i = 1; i <= NUM_BLOCKS; i++)
	{
		TEST_ASSERT((numbers = nanvix_rcache_alloc()) != RMEM_NULL);

		TEST_ASSERT((cached_data = nanvix_rcache_get(numbers)) != NULL);
		for (unsigned j = 0; j < RMEM_BLOCK_SIZE/sizeof(unsigned); j++)
			cached_data[j] = (i - 1)*RMEM_NUM_BLOCKS + j;

		TEST_ASSERT(nanvix_rcache_flush(numbers) == 0);
		TEST_ASSERT(nanvix_rcache_put(numbers, 0) == 0);

		TEST_ASSERT((cached_data = nanvix_rcache_get(numbers)) != NULL);

		for (unsigned j = 0; j < RMEM_BLOCK_SIZE/sizeof(unsigned); j++)
			TEST_ASSERT(cached_data[j] == (i - 1)*RMEM_NUM_BLOCKS + j);

		TEST_ASSERT(nanvix_rcache_free(numbers) == 0);
	}
}

/*============================================================================*
 * Stress Test: Consistency                                                   *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency 2-Step
 */
static void test_rmem_rcache_consistency2(void)
{
	rpage_t numbers[NUM_BLOCKS];
	unsigned *cached_data;

	for (unsigned i = 1; i <= NUM_BLOCKS; i++)
	{
		TEST_ASSERT((numbers[i - 1] = nanvix_rcache_alloc()) != RMEM_NULL);

		TEST_ASSERT((cached_data = nanvix_rcache_get(numbers[i - 1])) != NULL);
		for (unsigned j = 0; j < RMEM_BLOCK_SIZE/sizeof(unsigned); j++)
			cached_data[j] = (i - 1)*RMEM_NUM_BLOCKS + j;

	}

	for (unsigned i = 1; i <= NUM_BLOCKS; i++)
	{
		TEST_ASSERT((cached_data = nanvix_rcache_get(numbers[i - 1])) != NULL);

		for (unsigned j = 0; j < RMEM_BLOCK_SIZE/sizeof(unsigned); j++)
			TEST_ASSERT(cached_data[j] == (i - 1)*RMEM_NUM_BLOCKS + j);

		TEST_ASSERT(nanvix_rcache_free(numbers[i - 1]) == 0);
	}
}

/*============================================================================*
 * Test Driver Table                                                          *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_cache_stress[] = {
	{ test_rmem_rcache_consistency,  "consistency       " },
	{ test_rmem_rcache_consistency2, "consistency 2-step" },
	{ NULL,                           NULL                },
};
