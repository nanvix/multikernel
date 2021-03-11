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
#define __NEED_NAME_CLIENT
#define __NEED_NAME_SERVICE

#include <nanvix/runtime/pm/name.h>
#include <nanvix/sys/noc.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include "../test.h"

/*============================================================================*
 * API Test: Link Unlink                                                      *
 *============================================================================*/

/**
 * @brief API Test: Link Unlink
 */
static void test_name_link_unlink(void)
{
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = knode_get_num();

	ustrcpy(pathname, "cool-name");
	TEST_ASSERT(nanvix_name_link(nodenum, pathname) == 0);
	TEST_ASSERT(nanvix_name_unlink(pathname) == 0);
}

/*============================================================================*
 * API Test: Double Link                                                      *
 *============================================================================*/

/**
 * @brief API Test: Double Link
 */
static void test_name_double_link(void)
{
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = knode_get_num();

	/* Link name. */
	ustrcpy(pathname, "cool-name");
	TEST_ASSERT(nanvix_name_link(nodenum, pathname) == 0);
	TEST_ASSERT(nanvix_name_link(nodenum, pathname) == 0);
	TEST_ASSERT(nanvix_name_unlink(pathname) == 0);
	TEST_ASSERT(nanvix_name_unlink(pathname) == 0);
}

/*============================================================================*
 * API Test: Lookup                                                           *
 *============================================================================*/

/**
 * @brief API Test: Lookup
 */
static void test_name_lookup(void)
{
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = knode_get_num();

	ustrcpy(pathname, "cool-name");
	TEST_ASSERT(nanvix_name_link(nodenum, pathname) == 0);
	TEST_ASSERT(nanvix_name_lookup(pathname) == nodenum);
	TEST_ASSERT(nanvix_name_unlink(pathname) == 0);
}

/*============================================================================*
 * API Test: Heartbeat                                                        *
 *============================================================================*/

/**
 * @brief API Test: Lookup
 */
static void test_name_heartbeat(void)
{
	int nodenum;
	char pathname[NANVIX_PROC_NAME_MAX];

	nodenum = knode_get_num();

	ustrcpy(pathname, "cool-name");
	TEST_ASSERT(nanvix_name_link(nodenum, pathname) == 0);
	TEST_ASSERT(nanvix_name_heartbeat() == 0);
	TEST_ASSERT(nanvix_name_unlink(pathname) == 0);
}

/*============================================================================*
 * API Test: Register Unregister                                              *
 *============================================================================*/

/**
 * @brief API Test: Register / Unregister
 */
static void test_name_register_unregister(void)
{
	char name[NANVIX_PROC_NAME_MAX];

	ustrcpy(name, "cool-name");

	TEST_ASSERT(nanvix_name_register(name, 0) == 0);
	TEST_ASSERT(nanvix_name_unregister(name) == 0);
}

/*============================================================================*
 * API Test: Local Address Lookup                                             *
 *============================================================================*/

/**
 * @brief API Test: Local Address Lookup
 */
static void test_name_local_address_lookup(void)
{
	int port_nr;
	int test_ret;
	char name[NANVIX_PROC_NAME_MAX];

	port_nr = 0;
	test_ret = -1;
	ustrcpy(name, "cool-name");

	TEST_ASSERT(nanvix_name_register(name, port_nr) == 0);

	TEST_ASSERT(nanvix_name_address_lookup(name, &test_ret) == knode_get_num());
	TEST_ASSERT(test_ret == port_nr);

	TEST_ASSERT(nanvix_name_unregister(name) == 0);
}

/*============================================================================*
 * API Test Driver Table                                                      *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_name_api[] = {
	{ test_name_link_unlink,           "link unlink"         },
	{ test_name_double_link,           "double link"         },
	{ test_name_lookup,                "lookup"              },
	{ test_name_heartbeat,             "heartbeat"           },
	{ test_name_register_unregister,   "register unregister" },
	{ test_name_local_address_lookup,  "local addr lookup"   },
	{ NULL,                             NULL                 }
};
