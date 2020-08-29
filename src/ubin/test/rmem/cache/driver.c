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

#define __NEED_MM_RCACHE

#include <nanvix/runtime/mm.h>
#include <nanvix/ulib.h>
#include "../../test.h"

/* Import definitions. */
extern struct test tests_rmem_cache_api[];
extern struct test tests_rmem_cache_stress[];

/**
 * @todo TODO: provide a detailed description for this function.
 */
void test_rmem_cache(void)
{
	int policies[] = { RCACHE_BYPASS, RCACHE_FIFO, -1 };

	for (int j = 0; policies[j] >= 0; j++)
	{
		nanvix_rcache_select_replacement_policy(policies[j]);

		/* Run API tests. */
		for (int i = 0; tests_rmem_cache_api[i].test_fn != NULL; i++)
		{
			uprintf("[nanvix][test][rmem][cache][api] %s", tests_rmem_cache_api[i].name);
			tests_rmem_cache_api[i].test_fn();
		}

		/* Run stress tests. */
		for (int i = 0; tests_rmem_cache_stress[i].test_fn != NULL; i++)
		{
			uprintf("[nanvix][test][rmem][cache][stress] %s", tests_rmem_cache_stress[i].name);
			tests_rmem_cache_stress[i].test_fn();
		}
	}

	nanvix_rcache_select_replacement_policy(__RCACHE_DEFAULT_REPLACEMENT);
}
