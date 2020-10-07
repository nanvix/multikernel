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
#define __NEED_MM_RMEM_STUB
#define __NEED_MM_RCACHE

#ifndef UPDATE_FREQ
#define UPDATE_FREQ 10
#endif

#include <nanvix/runtime/mm.h>
#include <nanvix/config.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/**
 * @brief Page Cache
 */
static struct
{
	int initialized;     /**< Initialized?      */
	int update_frequency;
	int (*evict_fn)(void); /**< Eviction Strategy */
	bool nfu_update;
	bool aging_update;

	/**
	 * @brief Statistics
	 */
	struct
	{
		unsigned ngets;   /**< Number of Cache Gets */
		unsigned nmisses; /**< Number of Misses     */
		unsigned nhits;   /**< Number of Hits       */
	} stats;

	/**
	 * @brief Lines
	 */
	struct
	{
		int age;
		rpage_t pgnum;
		int refbit;
		int refcount;
		char page[RMEM_BLOCK_SIZE] ALIGN(PAGE_SIZE);
	} lines[RCACHE_LENGTH];
} cache;

/**
 * @brief Initializes a cache entry.
 *
 * @param x Index of target cache entry.
 */
#define CACHE_ENTRY_INITIALIZER(x) {       \
		cache.lines[x].refcount = 0;       \
		cache.lines[x].refbit = 0;		   \
		cache.lines[x].pgnum = RMEM_NULL;  \
}


/*============================================================================*
 * nanvix_rcache_flush()                                                      *
 *============================================================================*/

/**
 * @brief Flushes changes on a remote page.
 *
 * @param idx Target line index.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure a negative error code is returned instead.
 */
static int nanvix_rcache_flush(int idx)
{
	int err; /* Error Code */

	/* Write page back to remote memory. */
	if ((err = nanvix_rmem_write(cache.lines[idx].pgnum, cache.lines[idx].page)) < 0)
		return (err);

	return (0);
}

/*============================================================================*
 * nanvix_rcache_page_search()                                                *
 *============================================================================*/

/**
 * @brief Searches for a page in the cache.
 *
 * @param pgnum Number of the target page.
 *
 * @returns Upon successful completion, the page index is returned. Upon
 * failure a negative error code is returned instead.
 */
static int nanvix_rcache_page_search(rpage_t pgnum)
{
	for (int i = 0; i < RCACHE_LENGTH; i++)
	{
		/* Found. */
		if (cache.lines[i].pgnum == pgnum)
			return (i);
	}

	return (-ENOENT);
}

/*============================================================================*
 * nanvix_rcache_alloc_entry()                                                *
 *============================================================================*/

/**
 * @brief Looks for an empty entry in the cache.
 *
 * @returns The index of an empty entry in the cache, or a negative
 * number if no such entry exists.
 */
static int nanvix_rcache_empty(void)
{
	/* Get an empty entry. */
	for (int i = 0; i < RCACHE_LENGTH; i++)
	{
		if (cache.lines[i].pgnum == RMEM_NULL)
			return (i);
	}

	return (-1);
}

/*============================================================================*
 * nanvix_rcache_bypass()                                                     *
 *============================================================================*/

/**
 * @brief Bypasses the cache.
 *
 * @returns Upon successful completion, the index of the page is
 * returned. Upon failure a negative error code is returned instead.
 */
static int nanvix_rcache_bypass(void)
{
	int idx;

	/* Use this entry always. */
	idx = 0;

	/* Write back entry. */
	nanvix_rcache_flush(idx);

	/* Update entry. */
	CACHE_ENTRY_INITIALIZER(idx);

	return (idx);
}

/*============================================================================*
 * nanvix_rcache_fifo()                                                       *
 *============================================================================*/

/**
 * @brief Evicts a page using FIFO replacement policy.
 *
 * @returns Upon successful completion, the index of the page is
 * returned. Upon failure a negative error code is returned instead.
 */
static int nanvix_rcache_fifo(void)
{
	int idx;
	int old_entry;

	/* Get an empty entry. */
	if ((idx = nanvix_rcache_empty()) >= 0)
		return (idx);

	/* Find the oldest entry. */
	idx = 0;
	for (int i = 1; i < RCACHE_LENGTH; i++)
	{
		if (cache.lines[i].age < old_entry)
		{
			old_entry = cache.lines[i].age;
			idx = i;
		}
	}

	/* Write back entry. */
	nanvix_rcache_flush(idx);

	/* Update entry. */
	CACHE_ENTRY_INITIALIZER(idx);

	return (idx);
}

static int nanvix_rcache_nfu(void)
{
	return nanvix_rcache_fifo();
}

static int nanvix_rcache_aging(void)
{
	return nanvix_rcache_fifo();
}

/*============================================================================*
 * nanvix_rcache_select_replacement_policy()                                  *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_select_replacement_policy(int num)
{
	switch (num)
	{
		case RCACHE_BYPASS:
			cache.evict_fn = nanvix_rcache_bypass;
			cache.update_frequency = 1;
			break;

		case RCACHE_FIFO:
			cache.evict_fn = nanvix_rcache_fifo;
			cache.update_frequency = 1;
			break;

		case RCACHE_NFU:
			cache.evict_fn = nanvix_rcache_nfu;
			cache.update_frequency = UPDATE_FREQ;
			cache.nfu_update = true;
			break;

		case RCACHE_AGING:
			cache.evict_fn = nanvix_rcache_aging;
			cache.update_frequency = UPDATE_FREQ;
			cache.aging_update = true;
			break;

		default:
			uprintf("[nanvix][rcache] unknown replacement policy");
			uprintf("[nanvix][rcache] falling back to bypass mode");
			cache.evict_fn = nanvix_rcache_bypass;
			return (-EINVAL);
	}

	return (0);
}

/*============================================================================*
 * nanvix_rcache_ralloc()                                                     *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
rpage_t nanvix_rcache_alloc(void)
{
	rpage_t pgnum;

	/* Forward allocation to remote memory. */
	if ((pgnum = nanvix_rmem_alloc()) == RMEM_NULL)
		return (RMEM_NULL);

	return (pgnum);
}

/*============================================================================*
 * nanvix_rcache_free()                                                       *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_free(rpage_t pgnum)
{
	/* Invalid page number. */
	if (pgnum == RMEM_NULL)
		return (-EINVAL);

	return (nanvix_rmem_free(pgnum));
}

/*============================================================================*
 * nanvix_rcache_reference_update()                                           *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
void nanvix_rcache_reference_update(void)
{
	if (cache.stats.ngets%cache.update_frequency == 0)
	{
		if (cache.nfu_update == true)
		{
			for (int i = 0; i < RCACHE_LENGTH; i++)
				if (cache.lines[i].refbit == 1)
				{
					cache.lines[i].age++;
					cache.lines[i].refbit = 0;
				}
		}
		else if (cache.aging_update == true)
		{
			for (int i = 0; i < RCACHE_LENGTH; i++)
			{
				cache.lines[i].age = ((cache.lines[i].refbit << (sizeof(int)*8-1)) | (cache.lines[i].age >> 1));
				cache.lines[i].refbit = 0;
			}
		}
	}

}

/*============================================================================*
 * nanvix_rcache_get()                                                        *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
void *nanvix_rcache_get(rpage_t pgnum)
{
	int idx;

	/* Invalid page number. */
	if (pgnum == RMEM_NULL)
		return (NULL);

	/* Search the cache. */
	if((idx = nanvix_rcache_page_search(pgnum)) < 0)
	{
		int err;

		/* Evit a page. */
		if ((idx = cache.evict_fn()) < 0)
			return (NULL);

		/* Load page remote page. */
		if ((err = nanvix_rmem_read(pgnum, cache.lines[idx].page)) < 0)
			return (NULL);

		/* Update entry.*/
		cache.stats.nmisses++;

		cache.lines[idx].age = ((cache.nfu_update == true || cache.aging_update == true) ? 0 : cache.stats.ngets);
		cache.lines[idx].pgnum = pgnum;
	} else {
		cache.stats.nhits++;
	}

	cache.lines[idx].refbit |= 1;
	cache.lines[idx].refcount++;
	cache.stats.ngets++;

	nanvix_rcache_reference_update();


	uprintf("[cache] stats: %d hits, %d misses", cache.stats.nhits, cache.stats.nmisses);
	return (cache.lines[idx].page);
}

/*============================================================================*
 * nanvix_rcache_put()                                                        *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_put(rpage_t pgnum, int strike)
{
	int idx;

	((void) strike);

	/* Invalid page number. */
	if (pgnum == RMEM_NULL)
		return (-EINVAL);

	/* Search the cache. */
	if ((idx = nanvix_rcache_page_search(pgnum)) < 0)
		return (-ENOENT);

	/* Update entry.*/
	if (cache.lines[idx].refcount-- == 1)
		nanvix_rcache_flush(idx);

	return (0);
}

/*============================================================================*
 * nanvix_rcache_put()                                                        *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_clear_stats(void)
{
	cache.stats.nhits = 0;
	cache.stats.nmisses = 0;
	return (0);
}

/*============================================================================*
 * nanvix_rcache_setup()                                                      *
 *============================================================================*/

/**
 * The nanvix_rcache_setup() function initializes the page cache.
 */
int __nanvix_rcache_setup(void)
{
	/* Nothing to do. */
	if (cache.initialized)
		return (0);

	/* Initialize cache lines. */
	for (int i = 0; i < RCACHE_LENGTH; i++)
		CACHE_ENTRY_INITIALIZER(i);

	/* Initialize cache statistics. */
	cache.stats.ngets = 0;
	cache.stats.nmisses = 0;
	cache.stats.nhits = 0;

	cache.nfu_update = false;
	cache.aging_update = false;

	nanvix_rcache_select_replacement_policy(__RCACHE_DEFAULT_REPLACEMENT);

	cache.initialized = 1;

	uprintf("[nanvix][rcache] page cache initialized");

	return (0);
}
