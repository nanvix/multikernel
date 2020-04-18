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

#define __NEED_RMEM_CACHE

#include <nanvix/runtime/rmem.h>
#include <nanvix/ulib.h>
#include "../../test.h"


/*============================================================================*
 * Global declarations                                                        *
 *============================================================================*/

rpage_t page_num[(RMEM_CACHE_LENGTH+1)*RMEM_CACHE_BLOCK_SIZE];
char *cache_data;

/*============================================================================*
 * API Test: Alloc Free                                                       *
 *============================================================================*/

/**
 * @brief API Test: Alloc Free
 */
static void test_rmem_rcache_alloc_free(void)
{
	rpage_t page_num1;
	rpage_t page_num2;

	TEST_ASSERT((page_num1 = nanvix_rcache_alloc()) != RMEM_NULL);
	TEST_ASSERT((page_num2 = nanvix_rcache_alloc()) != RMEM_NULL);
	TEST_ASSERT(nanvix_rcache_free(page_num2) == 0);
	TEST_ASSERT(nanvix_rcache_free(page_num1) == 0);
	nanvix_rcache_clean();
}

/*============================================================================*
 * API Test: Cache Put Write                                                  *
 *============================================================================*/

/**
 * @brief API Test: Cache Put Write
 */
static void test_rmem_rcache_put_write(void)
{
	rpage_t page[RMEM_CACHE_BLOCK_SIZE];

	// Alloc for others blocks when we have block_size > 1
	for (int i = 0; i < RMEM_CACHE_BLOCK_SIZE; i++)
			TEST_ASSERT((page[i] = nanvix_rcache_alloc()) != RMEM_NULL);

	TEST_ASSERT((cache_data = nanvix_rcache_get(page[0])) != NULL);
	umemset(cache_data, 1, RMEM_BLOCK_SIZE);

	nanvix_rcache_select_write(RMEM_CACHE_WRITE_BACK);
	TEST_ASSERT(nanvix_rcache_put(page[0],0) == 0);
	TEST_ASSERT(nanvix_rcache_put(page[0],0) < 0);

	TEST_ASSERT((cache_data = nanvix_rcache_get(page[0])) != NULL);

	nanvix_rcache_select_write(RMEM_CACHE_WRITE_THROUGH);
	TEST_ASSERT(nanvix_rcache_put(page[0],0) == 0);
	TEST_ASSERT(nanvix_rcache_put(page[0],0) < 0);

	for (int i = 0; i < RMEM_CACHE_BLOCK_SIZE; i++)
			TEST_ASSERT((nanvix_rcache_free(page[i])) == 0);
	nanvix_rcache_clean();
}

/*============================================================================*
 * API Test: Cache Get Flush                                                  *
 *============================================================================*/

/**
 * @brief API Test: Cache Get Flush
 */
static void test_rmem_rcache_get_flush(void)
{

	/* Alloc every page available.
	 * The sum is for the last page RMEM_CACHE_BLOCK_SIZE (bad block problem).
	 */
	for (int i = 0; i < (RMEM_CACHE_LENGTH+1)*RMEM_CACHE_BLOCK_SIZE; i++)
		TEST_ASSERT((page_num[i] = nanvix_rcache_alloc()) != RMEM_NULL);

	/* Get and flush to every page possible except the last one allocated. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		for (int j = 0; j < RMEM_CACHE_BLOCK_SIZE; j++)
		{
			TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[i*RMEM_CACHE_BLOCK_SIZE+j])) != NULL);
			umemset(cache_data, i*RMEM_CACHE_BLOCK_SIZE+j, RMEM_BLOCK_SIZE);
		}
	}

	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
		TEST_ASSERT(nanvix_rcache_flush(page_num[i]) == 0);

	/* Evict to test flush on server side. */
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[RMEM_CACHE_LENGTH*RMEM_CACHE_BLOCK_SIZE])) != NULL);

	/* Check if every page has the correct value. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		for (int j = 0; j < RMEM_CACHE_BLOCK_SIZE; j++)
		{
			TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[i*RMEM_CACHE_BLOCK_SIZE+j])) != NULL);

			/* Checksum */
			for (size_t w = 0; w < RMEM_BLOCK_SIZE; w++)
				TEST_ASSERT(cache_data[w] == (char)(i*RMEM_CACHE_BLOCK_SIZE+j));
		}
	}

	/* Free all used pages. */
	for (int i = 0; i < (RMEM_CACHE_LENGTH+1)*RMEM_CACHE_BLOCK_SIZE; i++)
		TEST_ASSERT(nanvix_rcache_free(page_num[i]) == 0);

	nanvix_rcache_clean();
}

/*============================================================================*
 * API Test: Cache FiFo                                                       *
 *============================================================================*/

/**
 * @brief API Test: Cache FiFo
 */
static void test_rmem_rcache_fifo(void)
{

	nanvix_rcache_select_replacement_policy(RMEM_CACHE_FIFO);

	/* Allocate every page available plus one for evict purposes. */
	for (int i = 0; i < (RMEM_CACHE_LENGTH+1)*RMEM_CACHE_BLOCK_SIZE; i++)
		TEST_ASSERT((page_num[i] = nanvix_rcache_alloc()) != RMEM_NULL);

	/* Get every page to put it in the cache. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[i*RMEM_CACHE_BLOCK_SIZE])) != NULL);
		umemset(cache_data, i+1, RMEM_BLOCK_SIZE);
	}

	/* Flush every page to server. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
		TEST_ASSERT(nanvix_rcache_flush(page_num[i*RMEM_CACHE_BLOCK_SIZE]) == 0);

	/* Evict a page. */
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[RMEM_CACHE_LENGTH*RMEM_CACHE_BLOCK_SIZE])) != NULL);

	/* Check if the correct page was evicted */
	TEST_ASSERT(nanvix_rcache_flush(page_num[0*RMEM_CACHE_BLOCK_SIZE]) < 0);

	/* Free pages. */
	for (int i = 0; i < (RMEM_CACHE_LENGTH+1)*RMEM_CACHE_BLOCK_SIZE; i++)
		TEST_ASSERT(nanvix_rcache_free(page_num[i]) == 0);

	nanvix_rcache_clean();
}

/*============================================================================*
 * API Test: Cache LiFo                                                       *
 *============================================================================*/

/**
 * @brief API Test: Cache LiFo
 */
static void test_rmem_rcache_lifo(void)
{

	nanvix_rcache_select_replacement_policy(RMEM_CACHE_LIFO);

	/* Allocate every page available. */
	for (int i = 0; i < (RMEM_CACHE_LENGTH+1)*RMEM_CACHE_BLOCK_SIZE; i++)
		TEST_ASSERT((page_num[i] = nanvix_rcache_alloc()) != RMEM_NULL);

	/* Get every page to put it in the cache. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[i*RMEM_CACHE_BLOCK_SIZE])) != NULL);
		umemset(cache_data, i+1, RMEM_BLOCK_SIZE);
	}

	/* Flush every page to the server. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
		TEST_ASSERT(nanvix_rcache_flush(page_num[i*RMEM_CACHE_BLOCK_SIZE]) == 0);

	/* Eviction will occur */
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[RMEM_CACHE_LENGTH*RMEM_CACHE_BLOCK_SIZE])) != NULL);

	/* Check if the correct page was evicted */
	TEST_ASSERT(nanvix_rcache_flush(page_num[(RMEM_CACHE_LENGTH-1)*RMEM_CACHE_BLOCK_SIZE]) < 0);

	/* Free every used page. */
	for (int i = 0; i < (RMEM_CACHE_LENGTH+1)*RMEM_CACHE_BLOCK_SIZE; i++)
		TEST_ASSERT(nanvix_rcache_free(page_num[i]) == 0);

	nanvix_rcache_clean();
}

/*============================================================================*
 * API Test: Cache nfu                                                        *
 *============================================================================*/

/**
 * @brief API Test: Cache nfu
 */
static void test_rmem_rcache_nfu(void)
{

	nanvix_rcache_select_replacement_policy(RMEM_CACHE_NFU);

	/* Allocate every page available. */
	for (int i = 0; i < (RMEM_CACHE_LENGTH+1)*RMEM_CACHE_BLOCK_SIZE; i++)
		TEST_ASSERT((page_num[i] = nanvix_rcache_alloc()) != RMEM_NULL);

	/* Get every page to put it in the cache. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[i*RMEM_CACHE_BLOCK_SIZE])) != NULL);
		umemset(cache_data, i+1, RMEM_BLOCK_SIZE);
	}

	/* Flush every page to the server. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
		TEST_ASSERT(nanvix_rcache_flush(page_num[i]) == 0);

	/* Access all pages except one for controlled test. */
	int page_exception_value = RMEM_CACHE_LENGTH >> 1;
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		if (i != page_exception_value)
			TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[i*RMEM_CACHE_BLOCK_SIZE])) != NULL);
	}

	/* Eviction will occur */
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[RMEM_CACHE_LENGTH*RMEM_CACHE_BLOCK_SIZE])) != NULL);

	/* Check if the correct page was evicted */
	TEST_ASSERT(nanvix_rcache_flush(page_num[page_exception_value*RMEM_CACHE_BLOCK_SIZE]) < 0);

	/* Free every used page. */
	for (int i = 0; i < (RMEM_CACHE_LENGTH+1)*RMEM_CACHE_BLOCK_SIZE; i++)
		TEST_ASSERT(nanvix_rcache_free(page_num[i]) == 0);

	nanvix_rcache_clean();
}

/*============================================================================*
 * API Test: Cache Aging                                                      *
 *============================================================================*/

/**
 * @brief API Test: Cache Aging
 * Important: This test is a bit different from the others. We check if the page
 * was not evicted, instead of evicted. This behaviour is justified by the
 * age resolution. The test can not be generic without consider the age type and
 * the size of the cache with defines.
 */
static void test_rmem_rcache_aging(void)
{

	nanvix_rcache_select_replacement_policy(RMEM_CACHE_AGING);

	/* Allocate every page available. */
	for (int i = 0; i < (RMEM_CACHE_LENGTH+1)*RMEM_CACHE_BLOCK_SIZE; i++)
		TEST_ASSERT((page_num[i] = nanvix_rcache_alloc()) != RMEM_NULL);

	/* Get and flush to every page possible except the last one allocated. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[i*RMEM_CACHE_BLOCK_SIZE])) != NULL);
		umemset(cache_data, i+1, RMEM_BLOCK_SIZE);
	}

	/* Flush every page to the server. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
		TEST_ASSERT(nanvix_rcache_flush(page_num[i]) == 0);

	/* Get one page to control the test. */
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[0*RMEM_CACHE_BLOCK_SIZE])) != NULL);

	/* Eviction will occur */
	TEST_ASSERT((cache_data = nanvix_rcache_get(page_num[RMEM_CACHE_LENGTH*RMEM_CACHE_BLOCK_SIZE])) != NULL);

	/* Check if the correct page was evicted */
	TEST_ASSERT(nanvix_rcache_flush(page_num[0*RMEM_CACHE_BLOCK_SIZE]) == 0);

	/* Free every used page. */
	for (int i = 0; i < (RMEM_CACHE_LENGTH+1)*RMEM_CACHE_BLOCK_SIZE; i++)
		TEST_ASSERT(nanvix_rcache_free(page_num[i]) == 0);

	nanvix_rcache_clean();
}
/*============================================================================*
 * Test Driver Table                                                          *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_cache_api[] = {
	{ test_rmem_rcache_alloc_free, "alloc free" },
	{ test_rmem_rcache_put_write,  "put write"  },
	{ test_rmem_rcache_get_flush,  "get flush"  },
	{ test_rmem_rcache_fifo,       "fifo"       },
	{ test_rmem_rcache_lifo,       "lifo"       },
	{ test_rmem_rcache_nfu,        "nfu"        },
	{ test_rmem_rcache_aging,      "aging"      },
	{ NULL,                         NULL        },
};
