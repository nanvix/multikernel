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

#include <nanvix/config.h>
#include <nanvix/ulib.h>
#include "../test.h"

/* Import definitions. */
extern struct test tests_vfs_api[];
extern struct test tests_vfs_stress[];
extern struct test tests_vfs_fault[];

/**
 * @todo TODO: provide a detailed description for this function.
 */
void test_vfs(void)
{
#ifdef __NANVIX_HAS_VFS_SERVER

	/* Run api tests. */
	for (int i = 0; tests_vfs_api[i].test_fn != NULL; i++)
	{
		uprintf("[nanvix][test][vfs][api] %s", tests_vfs_api[i].name);
		tests_vfs_api[i].test_fn();
	}

	/* Run fault tests. */
	for (int i = 0; tests_vfs_fault[i].test_fn != NULL; i++)
	{
		uprintf("[nanvix][test][vfs][fault] %s", tests_vfs_fault[i].name);
		tests_vfs_fault[i].test_fn();
	}

	/* Run stress tests. */
	for (int i = 0; tests_vfs_stress[i].test_fn != NULL; i++)
	{
		uprintf("[nanvix][test][vfs][stress] %s", tests_vfs_stress[i].name);
		tests_vfs_stress[i].test_fn();
	}
#endif
}
