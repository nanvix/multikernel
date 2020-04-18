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
 * @brief Maximum value of unsigned char.
 */
#define UCHAR_MAX 255

/*============================================================================*
 * API Test: Alloc/Free                                                       *
 *============================================================================*/

/**
 * @brief API Test: Alloc/Free
 */
static void test_api_mem_alloc_free(void)
{
	unsigned char *ptr;

	for (unsigned i = 0; i < 4; i++)
	{
		TEST_ASSERT((ptr = nanvix_malloc(sizeof(unsigned char))) != NULL);
		nanvix_free(ptr);
	}
}

/*============================================================================*
 * API Test: Read/Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Read/Write
 */
static void test_api_mem_read_write(void)
{
	unsigned char *ptr;

	for (unsigned i = 0; i < 4; i++)
	{
		TEST_ASSERT((ptr = nanvix_malloc(sizeof(unsigned char))) != NULL);
		TEST_ASSERT((*ptr = UCHAR_MAX) == UCHAR_MAX);
		nanvix_free(ptr);
	}
}

/*============================================================================*
 * Stress Test: Read/Write                                                    *
 *============================================================================*/

/**
 * @brief Stress Test: Read/Write
 */
static void test_stress_mem_read_write(void)
{
	unsigned char *ptr;
	unsigned size = 2*RMEM_CACHE_SIZE*PAGE_SIZE;

	/* Allocate twice the size of the cache (in bytes). */
	TEST_ASSERT((ptr = nanvix_malloc(size)) != NULL);

	for (unsigned i = 0; i < size; i++)
		ptr[i] = i % (UCHAR_MAX + 1);

	for (unsigned i = 0; i < size; i++)
		TEST_ASSERT(ptr[i] == i % (UCHAR_MAX + 1));

	nanvix_free(ptr);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_mem_api[] = {
	{ test_api_mem_alloc_free,    "memory alloc/free" },
	{ test_api_mem_read_write,    "memory read/write" },
	{ test_stress_mem_read_write, "stress read/write" },
	{ NULL,                       NULL                },
};
