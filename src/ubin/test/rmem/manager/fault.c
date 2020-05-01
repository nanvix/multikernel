/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define __NEED_MM_MANAGER

#include <nanvix/runtime/mm.h>
#include "../../test.h"

/**
 * @brief Dummy buffer used for tests.
 */
static char buffer[RMEM_BLOCK_SIZE];

/*============================================================================*
 * Fault Injection Test: Invalid Alloc                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Alloc
 */
static void test_rmem_manager_invalid_alloc(void)
{
	TEST_ASSERT((nanvix_vmem_alloc(0)) == NULL);
	TEST_ASSERT((nanvix_vmem_alloc(RMEM_SIZE/RMEM_BLOCK_SHIFT + 1)) == NULL);
}

/*============================================================================*
 * Fault Injection Test: Invalid Free                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Free
 */
static void test_rmem_manager_invalid_free(void)
{
	TEST_ASSERT((nanvix_vmem_free(NULL)) < 0);
	TEST_ASSERT((nanvix_vmem_free(&test_rmem_manager_invalid_free)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Read                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void test_rmem_manager_invalid_read(void)
{
	void *ptr;

	TEST_ASSERT((ptr = nanvix_vmem_alloc(1)) != NULL);

		TEST_ASSERT(nanvix_vmem_read(NULL, ptr, RMEM_BLOCK_SIZE) == 0);
		TEST_ASSERT(nanvix_vmem_read(buffer, NULL, RMEM_BLOCK_SIZE) == 0);
		TEST_ASSERT(nanvix_vmem_read(buffer, ptr, RMEM_BLOCK_SIZE + 1) == 0);

	TEST_ASSERT(nanvix_vmem_free(ptr) == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Write                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Write
 */
static void test_rmem_manager_invalid_write(void)
{
	void *ptr;

	TEST_ASSERT((ptr = nanvix_vmem_alloc(1)) != NULL);

		TEST_ASSERT(nanvix_vmem_write(NULL, ptr, RMEM_BLOCK_SIZE) == 0);
		TEST_ASSERT(nanvix_vmem_write(NULL, buffer, RMEM_BLOCK_SIZE) == 0);
		TEST_ASSERT(nanvix_vmem_write(ptr, buffer, RMEM_BLOCK_SIZE + 1) == 0);

	TEST_ASSERT(nanvix_vmem_free(ptr) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_manager_fault[] = {
	{ test_rmem_manager_invalid_alloc, "invalid alloc" },
	{ test_rmem_manager_invalid_free,  "invalid free " },
	{ test_rmem_manager_invalid_read,  "invalid read " },
	{ test_rmem_manager_invalid_write, "invalid write" },
	{ NULL,                               NULL           },
};
