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

/* Must come first. */
#define __NEED_NAME_SERVICE
#define __NEED_NAME_CLIENT

#include <nanvix/runtime/pm/name.h>
#include <nanvix/sys/noc.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include "../test.h"

/*============================================================================*
 * Fault Injection Test: Invalid Link                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Link
 */
static void test_name_invalid_link(void)
{
	/* Link invalid names. */
	TEST_ASSERT(nanvix_name_link(-1, "missing_name") < 0);
	TEST_ASSERT(nanvix_name_link(1000000, "missing_name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Link                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Link
 */
static void test_name_bad_link(void)
{
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	nodenum = knode_get_num();

	umemset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Link invalid names. */
	TEST_ASSERT(nanvix_name_link(nodenum, pathname) < 0);
	TEST_ASSERT(nanvix_name_link(nodenum, NULL) < 0);
	TEST_ASSERT(nanvix_name_link(nodenum, "") < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Unlink                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Unlink
 */
static void test_name_invalid_unlink(void)
{
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	umemset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Unlink invalid names. */
	TEST_ASSERT(nanvix_name_unlink(pathname) < 0);
	TEST_ASSERT(nanvix_name_unlink(NULL) < 0);
	TEST_ASSERT(nanvix_name_unlink("") < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Unlink                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Unlink
 */
static void test_name_bad_unlink(void)
{
	int nodenum;

	nodenum = knode_get_num();

	/* Unlink missing name. */
	TEST_ASSERT(nanvix_name_link(nodenum, "cool-name") == 0);
	TEST_ASSERT(nanvix_name_unlink("missing_name") < 0);
	TEST_ASSERT(nanvix_name_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Double Unlink                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Unlink
 */
static void test_name_double_unlink(void)
{
	int nodenum;

	nodenum = knode_get_num();

	/* Unlink missing name. */
	TEST_ASSERT(nanvix_name_link(nodenum, "cool-name") == 0);
	TEST_ASSERT(nanvix_name_unlink("cool-name") == 0);
	TEST_ASSERT(nanvix_name_unlink("cool-name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Lookup                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Lookup
 */
static void test_name_bad_lookup(void)
{
	/* Lookup missing name. */
	TEST_ASSERT(nanvix_name_lookup("missing_name") < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Lookup                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Lookup Invalid Name
 */
static void test_name_invalid_lookup(void)
{
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	umemset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	/* Lookup invalid names. */
	TEST_ASSERT(nanvix_name_lookup(pathname) < 0);
	TEST_ASSERT(nanvix_name_lookup(NULL) < 0);
	TEST_ASSERT(nanvix_name_lookup("") < 0);
}


/*============================================================================*
 * Fault Injection Driver Table                                               *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_name_fault[] = {
	{ test_name_invalid_link,   "invalid link"   },
	{ test_name_bad_link,       "bad link"       },
	{ test_name_invalid_unlink, "invalid unlink" },
	{ test_name_bad_unlink,     "bad unlink"     },
	{ test_name_double_unlink,  "double unlink"  },
	{ test_name_invalid_lookup, "invalid lookup" },
	{ test_name_bad_lookup,     "bad lookup"     },
	{ NULL,                      NULL            },
};
