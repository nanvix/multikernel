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
#include <nanvix/sys/noc.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include "../test.h"

/*============================================================================*
 * Bug Test: Portal Ports                                                     *
 *============================================================================*/

#define PORT_NUM    10
#define BUFFER_SIZE 128

static char buffer[BUFFER_SIZE];

/**
 * @brief Bug Test: Portal Ports 
 * Issue: [ikc] Dependency on Portal ports that are automatically allocated #224
 */
static void test_bug_portal_ports(void)
{
	int nodenum;
	int inportal;
	int outportal;

	nodenum = knode_get_num();

	TEST_ASSERT((inportal = kportal_create(nodenum, PORT_NUM)) >= 0);
	TEST_ASSERT((outportal = kportal_open(nodenum, nodenum, PORT_NUM)) >= 0);

		TEST_ASSERT(kportal_write(outportal, buffer, BUFFER_SIZE) == BUFFER_SIZE);

		/* Read Issue #224 */
		TEST_ASSERT(kportal_allow(inportal, nodenum, PORT_NUM) >= 0);
		TEST_ASSERT(kportal_read(inportal, buffer, BUFFER_SIZE) == BUFFER_SIZE);

	TEST_ASSERT(kportal_close(outportal) == 0);
	TEST_ASSERT(kportal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test Driver Table                                                      *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_bugs[] = {
	{ test_bug_portal_ports, "Portal Ports" },
	{ NULL,                   NULL          }
};

