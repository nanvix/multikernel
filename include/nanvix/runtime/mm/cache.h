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

#ifndef NANVIX_RUNTIME_MM_CACHE_H_
#define NANVIX_RUNTIME_MM_CACHE_H_

#ifdef __NEED_MM_RCACHE

	#include <nanvix/servers/rmem.h>
	#include <nanvix/types/mm/rmem.h>

#endif /* __NEED_MM_RCACHE */

	/**
	 * @brief Length of the page cache.
	 */
	#ifndef __RCACHE_LENGTH
	#define RCACHE_LENGTH 32
	#endif

	/**
	 * @brief Cache Size (in entries)
	 */
	#define RCACHE_SIZE RCACHE_LENGTH

	/**
	 * @name Page replacement policies.
	 */
	/**@{*/
	#define RCACHE_BYPASS 0 /**< Bypass Mode        */
	/**@}*/

	/**
	 * @brief Default cache replacement policy.
	 */
	#ifndef __RCACHE_DEFAULT_REPLACEMENT
	#define __RCACHE_DEFAULT_REPLACEMENT RCACHE_BYPASS
	#endif

#ifdef __NEED_MM_RCACHE

	/**
	 * @brief Allocates a remote page.
	 *
	 * @returns Upon successful completion, the number of the newly
	 * allocated remote page is returned. Upon failure, @p RMEM_NULL
	 * is returned instead.
	 */
	extern rpage_t nanvix_rcache_alloc(void);

	/**
	 * @brief Frees a remote page.
	 *
	 * @param pgnum Number of the target page.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rcache_free(rpage_t pgnum);

	/**
	 * @brief Gets remote page.
	 *
	 * @param pgnum Number of the target page.
	 *
	 * @returns Upon successful completion, a pointer to a local
	 * mapping of the remote page is returned. Upon failure, a @p
	 * NULL pointer is returned instead.
	 */
	extern void *nanvix_rcache_get(rpage_t pgnum);

	/**
	 * @brief Puts remote page.
	 *
	 * @param pgnum Number of the target page.
	 *
	 * @returns Upon successful completion, zero is returned. Upon failure a
	 * negative error code is returned instead.
	 */
	extern int nanvix_rcache_put(rpage_t pgnum, int strike);


	/**
	 * @brief Selects the cache replacement_policy.
	 *
	 * @param num Number of the replacement policy.
	 */
	extern int nanvix_rcache_select_replacement_policy(int num);

#endif /* __NEED_MM_RCACHE */

#endif /* NANVIX_RUNTIME_MM_CACHE_H_ */

