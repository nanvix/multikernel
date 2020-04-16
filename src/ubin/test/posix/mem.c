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

#include <nanvix/runtime/rmem.h>
#include <nanvix/ulib.h>
#include "../test.h"

/* Import definitions. */
extern void *nanvix_malloc(size_t size);
extern void nanvix_free(void *ptr);

/**
 * @brief Number of blocks for stress tests.
 */
#define NUM_BLOCKS 256

/**
 * @brief Number of pages for stress tests.
 */
#define NUM_PAGES (4*RMEM_CACHE_SIZE*PAGE_SIZE)

/**
 * @brief Magic number.
 */
const unsigned MAGIC = 0xdeadbeef;

/*============================================================================*
 * API Test: Alloc/Free                                                       *
 *============================================================================*/

/**
 * @brief API Test: Alloc/Free
 */
static void test_api_mem_alloc_free(void)
{
	unsigned *ptr;
	unsigned *ptr2;

	TEST_ASSERT((ptr = nanvix_malloc(sizeof(unsigned))) != NULL);
	nanvix_free(ptr);
	TEST_ASSERT((ptr2 = nanvix_malloc(sizeof(unsigned))) != NULL);
	nanvix_free(ptr2);
}

/*============================================================================*
 * API Test: Read/Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Read/Write
 */
static void test_api_mem_read_write(void)
{
	unsigned *ptr;

	TEST_ASSERT((ptr = nanvix_malloc(sizeof(unsigned))) != NULL);

	*ptr = MAGIC;

	/* Checksum. */
	TEST_ASSERT(*ptr == MAGIC);

	nanvix_free(ptr);
}

/*============================================================================*
 * Stress Test: Consistency                                                   *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency
 */
static void test_stress_mem_consistency(void)
{
	unsigned *ptr;
	unsigned length = NUM_PAGES*(PAGE_SIZE/sizeof(unsigned));
	size_t size = length*sizeof(unsigned);

	TEST_ASSERT((ptr = nanvix_malloc(size)) != NULL);

	for (unsigned i = 0; i < length; i++)
	{
		ptr[i] = i;
		TEST_ASSERT(ptr[i] == i);
	}

	nanvix_free(ptr);
}

/*============================================================================*
 * Stress Test: Consistency 2-Step                                            *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency 2-Step
 */
static void test_stress_mem_consistency2(void)
{
	unsigned *ptr;
	unsigned length = NUM_PAGES*(PAGE_SIZE/sizeof(unsigned));
	size_t size = length*sizeof(unsigned);

	TEST_ASSERT((ptr = nanvix_malloc(size)) != NULL);

	for (unsigned i = 0; i < length; i++)
		ptr[i] = i;

	for (unsigned i = 0; i < length; i++)
		TEST_ASSERT(ptr[i] == i);

	nanvix_free(ptr);
}

/*============================================================================*
 * Stress Test: Consistency 2-Step Reverse                                    *
 *============================================================================*/

/**
 * @brief Stress Test: Consistency 2-Step Reverse
 */
static void test_stress_mem_consistency2r(void)
{
	unsigned *ptr;
	unsigned length = NUM_PAGES*(PAGE_SIZE/sizeof(unsigned));
	size_t size = length*sizeof(unsigned);

	TEST_ASSERT((ptr = nanvix_malloc(size)) != NULL);

	for (unsigned i = 0; i < length; i++)
		buffer[i] = i;

	for (unsigned i = 0; i < length; i++)
		TEST_ASSERT(buffer[(length - 1) - i] == (length - i);

	nanvix_free(buffer);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_mem_api[] = {
	{ test_api_mem_alloc_free,       "memory alloc/free         "  },
	{ test_api_mem_read_write,       "memory read/write         "  },
	{ test_stress_mem_consistency,   "consistency               "  },
	{ test_stress_mem_consistency2,  "consistency 2-step        "  },
	{ test_stress_mem_consistency2r, "consistency 2-step reverse"  },
	{ NULL,                          NULL                          },
};
