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

#define __NEED_MM_RCACHE

#include <nanvix/runtime/mm.h>
#include <nanvix/ulib.h>
#include "../../test.h"

/**
 * @brief Number of blocks to allocate.
 */
#define NUM_BLOCKS (4)

/**
 * @brief Page numbers used in tests.
 */
rpage_t pgnums[NUM_BLOCKS];

/*============================================================================*
 * Stress Test: Alloc Free                                                   *
 *============================================================================*/

/**
 * @brief Stress Test: Alloc Free
 */
static void test_rmem_rcache_alloc_free(void)
{
	rpage_t pgnum;

	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		TEST_ASSERT((pgnum = nanvix_rcache_alloc()) != RMEM_NULL);
		TEST_ASSERT(nanvix_rcache_free(pgnum) == 0);
	}
}

/*============================================================================*
 * Stress Test: Alloc Free                                                    *
 *============================================================================*/

/**
 * @brief Stress Test: Alloc Free 2-Step
 */
static void test_rmem_rcache_alloc_free2(void)
{
	for (int i = 0; i < NUM_BLOCKS; i++)
		TEST_ASSERT((pgnums[i] = nanvix_rcache_alloc()) != RMEM_NULL);

	for (int i = 0; i < NUM_BLOCKS; i++)
		TEST_ASSERT(nanvix_rcache_free(pgnums[i]) == 0);
}

/*============================================================================*
 * Stress Test: Get Put                                                       *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency
 */
static void test_rmem_rcache_get_put(void)
{
	rpage_t pgnum;

	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		TEST_ASSERT((pgnum = nanvix_rcache_alloc()) != RMEM_NULL);

			TEST_ASSERT(nanvix_rcache_get(pgnum) != NULL);
			TEST_ASSERT(nanvix_rcache_put(pgnum, 0) == 0);

		TEST_ASSERT(nanvix_rcache_free(pgnum) == 0);
	}
}

/*============================================================================*
 * Stress Test: Get Put 2-Step                                                *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency 2-Step
 */
static void test_rmem_rcache_get_put2(void)
{
	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		TEST_ASSERT((pgnums[i] = nanvix_rcache_alloc()) != RMEM_NULL);

			TEST_ASSERT(nanvix_rcache_get(pgnums[i]) != NULL);
	}

	for (int i = 0; i < NUM_BLOCKS; i++)
	{
			nanvix_rcache_put(pgnums[i], 0);

		TEST_ASSERT(nanvix_rcache_free(pgnums[i]) == 0);
	}
}

/*============================================================================*
 * Stress Test: Consistency                                                   *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency
 */
static void test_rmem_rcache_consistency(void)
{
	char *page;

	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		TEST_ASSERT((pgnums[i] = nanvix_rcache_alloc()) != RMEM_NULL);

			TEST_ASSERT((page = nanvix_rcache_get(pgnums[i])) != NULL);
			umemset(page, i, RMEM_BLOCK_SIZE);
	}

	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		TEST_ASSERT((page = nanvix_rcache_get(pgnums[i])) != NULL);
		for (int j = 0; j < RMEM_BLOCK_SIZE; j++)
			TEST_ASSERT(page[j] == i);
	}

	for (int i = 0; i < NUM_BLOCKS; i++)
	{
			nanvix_rcache_put(pgnums[i], 0);

		TEST_ASSERT(nanvix_rcache_free(pgnums[i]) == 0);
	}
}

/*============================================================================*
 * Test Driver Table                                                          *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_cache_stress[] = {
	{ test_rmem_rcache_alloc_free,   "alloc free        " },
	{ test_rmem_rcache_alloc_free2,  "alloc free 2-step " },
	{ test_rmem_rcache_get_put,      "get put           " },
	{ test_rmem_rcache_get_put2,     "get put 2-step    " },
	{ test_rmem_rcache_consistency,  "consistency       " },
	{ NULL,                           NULL                },
};
