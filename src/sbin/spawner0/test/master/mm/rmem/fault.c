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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <nanvix/syscalls.h>
#include <nanvix/mm.h>
#include <nanvix/limits.h>

#include "test.h"

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/*============================================================================*
 * API Test: Invalid Write                                                    *
 *============================================================================*/

/**
 * @brief API Test: Invalid Write
 */
static void test_mm_rmem_invalid_write(void)
{
	char buffer[DATA_SIZE];

	memset(buffer, 1, DATA_SIZE);
	TEST_ASSERT(memwrite(RMEM_SIZE, buffer, DATA_SIZE) < 0);
	TEST_ASSERT(memwrite(RMEM_SIZE - DATA_SIZE/2, buffer, DATA_SIZE) < 0);
}

/*============================================================================*
 * API Test: Null Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Null Write
 */
static void test_mm_rmem_null_write(void)
{
	TEST_ASSERT(memwrite(0, NULL, DATA_SIZE) < 0);
}

/*============================================================================*
 * API Test: Invalid Write Size                                               *
 *============================================================================*/

/**
 * @brief API Test: Invalid Write Size
 */
static void test_mm_rmem_invalid_write_size(void)
{
	char buffer[DATA_SIZE];

	memset(buffer, 1, DATA_SIZE);
	TEST_ASSERT(memwrite(0, buffer, RMEM_BLOCK_SIZE + 1) < 0);
}

/*============================================================================*/

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test mm_rmem_tests_fault[] = {
	{ test_mm_rmem_invalid_write,      "Invalid Write"      },
	{ test_mm_rmem_null_write,         "Null Write"         },
	{ test_mm_rmem_invalid_write_size, "Invalid Write Size" },
	{ NULL,                            NULL                 },
};
