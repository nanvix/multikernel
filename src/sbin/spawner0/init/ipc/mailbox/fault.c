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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <mppaipc.h>

#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "test.h"

/*============================================================================*
 * API Test: Invalid Create                                                   *
 *============================================================================*/

/**
 * @brief API Test: Invalid Create
 */
static void test_ipc_mailbox_invalid_create(void)
{
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	TEST_ASSERT(mailbox_create(NULL) < 0);
	TEST_ASSERT(mailbox_create("") < 0);
	TEST_ASSERT(mailbox_create(pathname) < 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test ipc_mailbox_tests_fault[] = {
	{ test_ipc_mailbox_invalid_create, "Invalid Create" },
	{ NULL,                            NULL             },
};
