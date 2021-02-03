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

#include <nanvix/runtime/pm.h>
#include "../test.h"


/**
 * @brief API test: set pid and get pid
 */
static void test_proc_api_pid(void)
{
	TEST_ASSERT(nanvix_setpid() == 0);
	TEST_ASSERT(nanvix_getpid() > 0);
}

/**
 * @brief API test: set process group id
 */
static void test_proc_api_gid(void)
{
	pid_t pid;
	pid = nanvix_getpid();
	TEST_ASSERT(nanvix_getpgid(0) < 0);
	TEST_ASSERT(nanvix_getpgid(pid) < 0);
	TEST_ASSERT(nanvix_setpgid(0, 0) == 0);
	TEST_ASSERT(nanvix_getpgid(pid) == pid);
	TEST_ASSERT(nanvix_getpgid(0) == pid);
}

/**
 * @brief Unit tests.
 */
struct test tests_proc_api[] = {
	{ test_proc_api_pid,           "set get pid"    },
	{ test_proc_api_gid,           "set get group"    },
	{ NULL,                   NULL            }
};
