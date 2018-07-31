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

#include <string.h>
#include <stdlib.h>

#include <nanvix/syscalls.h>

#include "test.h"

/*============================================================================*
 * Fault Injection Test: Invalid Create                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Create
 */
static void test_sys_portal_invalid_create(void)
{
	int inportal;

	TEST_ASSERT((inportal = sys_portal_create(-1)) < 0);
	TEST_ASSERT((inportal = sys_portal_create(1000000)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Create                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Create
 */
static void test_sys_portal_bad_create(void)
{
	int inportal;

	TEST_ASSERT((inportal = sys_portal_create(0)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Create                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Create
 */
static void test_sys_portal_double_create(void)
{
	int inportal;
	int inportal2;
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((inportal = sys_portal_create(nodenum)) >= 0);
	TEST_ASSERT((inportal2 = sys_portal_create(nodenum)) < 0);
	TEST_ASSERT((sys_portal_unlink(inportal)) == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Open                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Open
 */
static void test_sys_portal_invalid_open(void)
{
	int outportal;

	TEST_ASSERT((outportal = sys_portal_open(-1)) < 0);
	TEST_ASSERT((outportal = sys_portal_open(1000000)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Open                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Open
 */
static void test_sys_portal_bad_open(void)
{
	int outportal;
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((outportal = sys_portal_open(nodenum)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Double Open                                          *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Open
 */
static void test_sys_portal_double_open(void)
{
	int outportal;
	int outportal2;

	TEST_ASSERT((outportal = sys_portal_open(0)) >= 0);
	TEST_ASSERT((outportal2 = sys_portal_open(0)) < 0);
	TEST_ASSERT((sys_portal_close(outportal)) == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Unlink                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Unlink
 */
static void test_sys_portal_invalid_unlink(void)
{
	TEST_ASSERT(sys_portal_unlink(-1) < 0);
	TEST_ASSERT(sys_portal_unlink(1000000) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Unlink                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Unlink
 */
static void test_sys_portal_bad_unlink(void)
{
	int inportal;

	TEST_ASSERT(sys_portal_unlink(0) < 0);
	TEST_ASSERT((inportal = sys_portal_open(0)) >= 0);
	TEST_ASSERT(sys_portal_unlink(inportal) < 0);
	TEST_ASSERT((sys_portal_close(inportal)) == 0);
}

/*============================================================================*
 * Fault Injection Test: Double Unlink                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Unlink
 */
static void test_sys_portal_double_unlink(void)
{
	int inportal;
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT((inportal = sys_portal_create(nodenum)) >= 0);
	TEST_ASSERT((sys_portal_unlink(inportal)) == 0);
	TEST_ASSERT((sys_portal_unlink(inportal)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Close                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Close
 */
static void test_sys_portal_invalid_close(void)
{
	TEST_ASSERT(sys_portal_close(-1) < 0);
	TEST_ASSERT(sys_portal_close(1000000) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Close                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Close
 */
static void test_sys_portal_bad_close(void)
{
	int outportal;
	int nodenum;

	nodenum = sys_get_node_num();

	TEST_ASSERT(sys_portal_close(nodenum) < 0);
	TEST_ASSERT((outportal = sys_portal_create(nodenum)) >= 0);
	TEST_ASSERT(sys_portal_close(outportal) < 0);
	TEST_ASSERT((sys_portal_unlink(outportal)) == 0);
}

/*============================================================================*
 * Fault Injection Test: Double Close                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Double Close
 */
static void test_sys_portal_double_close(void)
{
	int outportal;

	TEST_ASSERT((outportal = sys_portal_open(0)) >= 0);
	TEST_ASSERT((sys_portal_close(outportal)) == 0);
	TEST_ASSERT((sys_portal_close(outportal)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Allow                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Allow
 */
static void test_sys_portal_invalid_allow(void)
{
	int nodenum;
	int inportal;

	TEST_ASSERT(sys_portal_allow(-1, 0) < 0);
	TEST_ASSERT(sys_portal_allow(1000000, 0) < 0);

	nodenum = sys_get_node_num();

	TEST_ASSERT((inportal = sys_portal_create(nodenum)) >= 0);
	TEST_ASSERT(sys_portal_allow(inportal, -1) < 0);
	TEST_ASSERT(sys_portal_allow(inportal, 1000000) < 0);
	TEST_ASSERT((sys_portal_unlink(inportal)) == 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Allow                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Allow
 */
static void test_sys_portal_bad_allow(void)
{
	int inportal;

	TEST_ASSERT(sys_portal_allow(0, 0) < 0);

	TEST_ASSERT((inportal = sys_portal_open(0)) >= 0);
	TEST_ASSERT(sys_portal_allow(inportal, 0) < 0);
	TEST_ASSERT((sys_portal_close(inportal)) == 0);
}

/*============================================================================*
 * Fault Injection Test: Double Allow                                         *
 *============================================================================*/

#ifdef _TEST_HAL_PORTAL_DOUBLE_ALLOW

/**
 * @brief Fault Injection Test: Double Allow
 */
static void test_sys_portal_double_allow(void)
{
	int nodenum;
	int inportal;

	nodenum = sys_get_node_num();

	TEST_ASSERT((inportal = sys_portal_create(nodenum)) >= 0);
	TEST_ASSERT(sys_portal_allow(inportal, 0) == 0);
	TEST_ASSERT(sys_portal_allow(inportal, 1) < 0);
	TEST_ASSERT((sys_portal_unlink(inportal)) == 0);
}

#endif

/*============================================================================*
 * Fault Injection Test: Invalid Read                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void test_sys_portal_invalid_read(void)
{
	int buf;

	TEST_ASSERT(sys_portal_read(-1, &buf, sizeof(buf)) < 0);
	TEST_ASSERT(sys_portal_read(1000000, &buf, sizeof(buf)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Read                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Read
 */
static void test_sys_portal_bad_read(void)
{
	int buf;
	int inportal;

	TEST_ASSERT(sys_portal_read(0, &buf, sizeof(buf)) < 0);
	TEST_ASSERT((inportal = sys_portal_open(0)) >= 0);
	TEST_ASSERT(sys_portal_read(inportal, &buf, sizeof(buf)) < 0);
	TEST_ASSERT(sys_portal_close(inportal) == 0);
}

/*============================================================================*
 * Fault Injection Test: Null Read                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Null Read
 */
static void test_sys_portal_null_read(void)
{
	int nodenum;
	int inportal;

	nodenum = sys_get_node_num();

	TEST_ASSERT((inportal = sys_portal_create(nodenum)) >= 0);
	TEST_ASSERT(sys_portal_read(inportal, NULL, 1) < 0);
	TEST_ASSERT(sys_portal_unlink(inportal) == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Write                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Write
 */
static void test_sys_portal_invalid_write(void)
{
	int buf;

	TEST_ASSERT(sys_portal_write(-1, &buf, sizeof(buf)) < 0);
	TEST_ASSERT(sys_portal_write(1000000, &buf, sizeof(buf)) < 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Write                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Write
 */
static void test_sys_portal_bad_write(void)
{
	int buf;
	int nodenum;
	int outportal;

	nodenum = sys_get_node_num();

	TEST_ASSERT(sys_portal_write(0, &buf, sizeof(buf)) < 0);
	TEST_ASSERT((outportal = sys_portal_create(nodenum)) >= 0);
	TEST_ASSERT(sys_portal_write(outportal, &buf, sizeof(buf)) < 0);
	TEST_ASSERT(sys_portal_unlink(outportal) == 0);
}

/*============================================================================*
 * Fault Injection Test: Null Write                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Null Write
 */
static void test_sys_portal_null_write(void)
{
	int outportal;

	TEST_ASSERT((outportal = sys_portal_open(0)) >= 0);
	TEST_ASSERT(sys_portal_write(outportal, NULL, 1) < 0);
	TEST_ASSERT(sys_portal_close(outportal) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test portal_tests_fault[] = {
	{ test_sys_portal_invalid_create, "Invalid Create" },
	{ test_sys_portal_bad_create,     "Bad Create"     },
	{ test_sys_portal_double_create,  "Double Create"  },
	{ test_sys_portal_invalid_open,   "Invalid Open"   },
	{ test_sys_portal_bad_open,       "Bad Open"       },
	{ test_sys_portal_double_open,    "Double Open"    },
	{ test_sys_portal_invalid_unlink, "Invalid Unlink" },
	{ test_sys_portal_double_unlink,  "Double Unlink"  },
	{ test_sys_portal_bad_unlink,     "Bad Unlink"     },
	{ test_sys_portal_invalid_close,  "Invalid Close"  },
	{ test_sys_portal_bad_close,      "Bad Close"      },
	{ test_sys_portal_double_close,   "Double Close"   },
	{ test_sys_portal_invalid_allow,  "Invalid Allow"  },
	{ test_sys_portal_bad_allow,      "Bad Allow"      },
#ifdef _TEST_HAL_PORTAL_DOUBLE_ALLOW
	{ test_sys_portal_double_allow,   "Double Allow"   },
#endif
	{ test_sys_portal_invalid_read,   "Invalid Read"   },
	{ test_sys_portal_bad_read,       "Bad Read"       },
	{ test_sys_portal_null_read,      "Null Read"      },
	{ test_sys_portal_invalid_write,  "Invalid Write"  },
	{ test_sys_portal_bad_write,      "Bad Write"      },
	{ test_sys_portal_null_write,     "Null Write"     },
	{ NULL,                           NULL             },
};