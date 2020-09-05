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


/*============================================================================*
 * Global declarations                                                        *
 *============================================================================*/

rpage_t page_num[RCACHE_SIZE];

/*============================================================================*
 * API Test: Alloc Free                                                       *
 *============================================================================*/

/**
 * @brief API Test: Alloc Free
 */
static void test_rmem_rcache_alloc_free(void)
{
	rpage_t pgnum;

	TEST_ASSERT((pgnum = nanvix_rcache_alloc()) != RMEM_NULL);
	TEST_ASSERT(nanvix_rcache_free(pgnum) == 0);
}

/*============================================================================*
 * API Test: Cache Put Write                                                  *
 *============================================================================*/

/**
 * @brief API Test: Cache Put Write
 */
static void test_rmem_rcache_put_write(void)
{
	char *page;
	rpage_t pgnum;

	TEST_ASSERT((pgnum = nanvix_rcache_alloc()) != RMEM_NULL);

		TEST_ASSERT((page = nanvix_rcache_get(pgnum)) != NULL);
		umemset(page, 1, RMEM_BLOCK_SIZE);
		TEST_ASSERT(nanvix_rcache_put(pgnum, 0) == 0);

	TEST_ASSERT(nanvix_rcache_free(pgnum) == 0);
}

/*============================================================================*
 * API Test: Cache Stats                                                      *
 *============================================================================*/

/**
 * @brief API Test: Cache Stats
 */
static void test_rmem_rcache_stats(void)
{
	char *page;
	rpage_t pgnum;
	struct rcache_stats stats1;
	struct rcache_stats stats2;

	TEST_ASSERT(nanvix_rcache_stats(&stats1) == 0);
	TEST_ASSERT((pgnum = nanvix_rcache_alloc()) != RMEM_NULL);
	TEST_ASSERT(nanvix_rcache_stats(&stats2) == 0);

	TEST_ASSERT((stats2.ngets - stats1.ngets) == 0);
	TEST_ASSERT((stats2.nmisses - stats1.nmisses) == 0);
	TEST_ASSERT((stats2.nhits - stats1.nhits) == 0);

		TEST_ASSERT(nanvix_rcache_stats(&stats1) == 0);
		TEST_ASSERT((page = nanvix_rcache_get(pgnum)) != NULL);
		TEST_ASSERT(nanvix_rcache_stats(&stats2) == 0);

		TEST_ASSERT((stats2.ngets - stats1.ngets) == 1);
		TEST_ASSERT((stats2.nmisses - stats1.nmisses) == 1);
		TEST_ASSERT((stats2.nhits - stats1.nhits) == 0);

		TEST_ASSERT(nanvix_rcache_stats(&stats1) == 0);
		TEST_ASSERT((page = nanvix_rcache_get(pgnum)) != NULL);
		TEST_ASSERT(nanvix_rcache_stats(&stats2) == 0);

		TEST_ASSERT((stats2.ngets - stats1.ngets) == 1);
		TEST_ASSERT((stats2.nmisses - stats1.nmisses) == 0);
		TEST_ASSERT((stats2.nhits - stats1.nhits) == 1);

		TEST_ASSERT(nanvix_rcache_stats(&stats1) == 0);
		TEST_ASSERT(nanvix_rcache_put(pgnum, 0) == 0);
		TEST_ASSERT(nanvix_rcache_stats(&stats2) == 0);

		TEST_ASSERT((stats2.ngets - stats1.ngets) == 0);
		TEST_ASSERT((stats2.nmisses - stats1.nmisses) == 0);
		TEST_ASSERT((stats2.nhits - stats1.nhits) == 0);

	TEST_ASSERT(nanvix_rcache_stats(&stats1) == 0);
	TEST_ASSERT(nanvix_rcache_free(pgnum) == 0);
	TEST_ASSERT(nanvix_rcache_stats(&stats2) == 0);

	TEST_ASSERT((stats2.ngets - stats1.ngets) == 0);
	TEST_ASSERT((stats2.nmisses - stats1.nmisses) == 0);
	TEST_ASSERT((stats2.nhits - stats1.nhits) == 0);
}

/*============================================================================*
 * Test Driver Table                                                          *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_cache_api[] = {
	{ test_rmem_rcache_put_write,  "get put   " },
	{ test_rmem_rcache_alloc_free, "alloc free" },
	{ test_rmem_rcache_stats,      "stats     " },
	{ NULL,                         NULL        },
};
