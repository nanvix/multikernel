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

#define __NEED_RMEM_SERVICE
#define __NEED_MM_STUB
#define __NEED_RMEM_CACHE

#include <nanvix/runtime/rmem.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/**
 * @brief Is the page cache initialized?
 */
static int initialized = 0;

/**
 * @brief Cache statistics.
 */
static struct
{
	unsigned nmisses; /**< Number of misses.     */
	unsigned nhits;   /**< Number of hits.       */
	unsigned nallocs; /**< Number of allocations */
} stats = { 0, 0, 0 };

#ifndef AGE_TYPE
	#define AGE_TYPE uint32_t
#endif

int update_count = 0;

#ifndef UPDATE_FREQ
	#define UPDATE_FREQ 1
#endif

/*
 * @brief Tuple structure.
 */
struct tuple
{
	int slot_idx;
	int block_idx;
	int error;
};

/**
 * @brief Cache slot representation.
 */
struct cache_slot
{
	rpage_t pgnum;
	char pages[RMEM_BLOCK_SIZE] ALIGN(PAGE_SIZE);
	AGE_TYPE age;
	int ref_count;
};

/*
 * @brief Page cache.
 */
static struct cache_slot cache_lines[RMEM_CACHE_SIZE] = {
	[0 ... ((RMEM_CACHE_SIZE) - 1)] = {.pgnum = RMEM_NULL, .age = 0, .ref_count = 0}
};

/**
 * @brief Discrete cache time.
 */
static unsigned cache_time = 0;

/**
 * @brief Default cache replacement policy.
 */
#ifndef __RMEM_CACHE_DEFAULT_REPLACEMENT
#define __RMEM_CACHE_DEFAULT_REPLACEMENT RMEM_CACHE_FIFO
#endif

/**
 * @brief Default cache write policy.
 */
#ifndef __RMEM_CACHE_DEFAULT_WRITE
#define __RMEM_CACHE_DEFAULT_WRITE RMEM_CACHE_WRITE_BACK
#endif

/**
 * @brief Cache replacement policy.
 */
static int cache_policy = __RMEM_CACHE_DEFAULT_REPLACEMENT;

/**
 * @brief Cache write policy.
 */
static int write_policy = __RMEM_CACHE_DEFAULT_WRITE;

/*============================================================================*
 * nanvix_rcache_clean()                                                      *
 *============================================================================*/

/**
 * @brief Cleans the cache.
 */
void nanvix_rcache_clean(void)
{
	for (int i = 0; i < RMEM_CACHE_LENGTH*RMEM_CACHE_BLOCK_SIZE; i++)
	{
		cache_lines[i].pgnum = RMEM_NULL;
		cache_lines[i].age = 0;
	}
}

/*============================================================================*
 * nanvix_rcache_page_search()                                                *
 *============================================================================*/

/**
 * @brief Searches for a page in the cache.
 *
 * @param pgnum Number of the target page.
 *
 * @returns Upon successful completion, the page index is
 * returned. Upon failure a negative error code is returned instead.
 */
static struct tuple nanvix_rcache_page_search(rpage_t pgnum)
{
	cache_time++;
	int pgnum_block;
	struct tuple indexes;

	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		pgnum_block = (int)(pgnum) - (int)(cache_lines[i*RMEM_CACHE_BLOCK_SIZE].pgnum);
		if (pgnum_block >= 0 && pgnum_block < RMEM_CACHE_BLOCK_SIZE && cache_lines[i*RMEM_CACHE_BLOCK_SIZE].pgnum != RMEM_NULL)
		{
			for (int j = 0; j < RMEM_CACHE_BLOCK_SIZE; j++)
			{
				if (cache_lines[i*RMEM_CACHE_BLOCK_SIZE+j].pgnum == pgnum)
				{
					indexes.slot_idx = i*RMEM_CACHE_BLOCK_SIZE;
					indexes.block_idx = j;
					indexes.error = 0;
					return indexes;
				}
			}
		}
	}

	indexes.error = (-EFAULT);
	return indexes;
}

static int nanvix_rcache_page_search_slot(rpage_t pgnum)
{
	cache_time++;
	int pgnum_block;

	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{ pgnum_block = (int)(pgnum) - (int)(cache_lines[i*RMEM_CACHE_BLOCK_SIZE].pgnum);
		if (pgnum_block >= 0 && pgnum_block < RMEM_CACHE_BLOCK_SIZE && cache_lines[i*RMEM_CACHE_BLOCK_SIZE].pgnum != RMEM_NULL)
		{
			for (int j = 0; j < RMEM_CACHE_BLOCK_SIZE; j++)
			{
				if (cache_lines[i*RMEM_CACHE_BLOCK_SIZE+j].pgnum == pgnum)
					return (i*RMEM_CACHE_BLOCK_SIZE);
			}
		}
	}

	return (-EFAULT);
}

static void nanvix_rcache_aging(int idx)
{
	AGE_TYPE temp_age = 0;
	update_count++;
	if (UPDATE_FREQ == update_count)
	{
		for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
		{
			temp_age = cache_lines[i*RMEM_CACHE_BLOCK_SIZE].age;
			temp_age = (temp_age) >> 1;
			if (i*RMEM_CACHE_BLOCK_SIZE == idx)
			{
				if (cache_lines[idx].ref_count == 1)
				{
					temp_age = (AGE_TYPE)1 << (sizeof(AGE_TYPE)*8-1) | temp_age;
					cache_lines[idx].ref_count = (UPDATE_FREQ == 1 ? 1 : 0);
				} else {
					temp_age = (AGE_TYPE)0 << (sizeof(AGE_TYPE)*8-1) | temp_age;
				}
			}
			cache_lines[i*RMEM_CACHE_BLOCK_SIZE].age = temp_age;
		}
		update_count = 0;
	} else {
		if (cache_lines[idx].ref_count == 0)
		{
			cache_lines[idx].ref_count++;
		}
	}
}

/*============================================================================*
 * nanvix_rcache_age_update_nfu()                                             *
 *============================================================================*/

/**
 * @brief Evicts pages from the cache based on the NFU replacement policy.
 *
 * @param pgnum Number of the target page.
 *
 * @returns Upon successful completion, the index of the page is
 * returned. Upon failure a negative error code is returned instead.
 */
static int nanvix_rcache_age_update(rpage_t pgnum)
{
	struct tuple idx = nanvix_rcache_page_search(pgnum);
	int slot = idx.slot_idx;
	int error = idx.error;

	cache_time++;

	if (error < 0)
		return (-EFAULT);

	if (cache_policy == RMEM_CACHE_NFU)
	{
		update_count++;
		if (UPDATE_FREQ == update_count)
		{
			if (cache_lines[slot].ref_count == 1)
			{
				cache_lines[slot].age++;
				cache_lines[slot].ref_count = (UPDATE_FREQ == 1 ? 1 : 0);
			}
			update_count = 0;
		} else {
			if (cache_lines[slot].ref_count == 0)
				cache_lines[slot].ref_count++;
		}
	} else if (cache_policy == RMEM_CACHE_AGING) {
		nanvix_rcache_aging(slot);
	}

	return (0);
}

/*============================================================================*
 * nanvix_rcache_age_init()                                                   *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
static int nanvix_rcache_age_init(rpage_t pgnum)
{
	struct tuple idx = nanvix_rcache_page_search(pgnum);
	int slot = idx.slot_idx;
	int error = idx.error;

	cache_time++;

	if (error < 0)
		return (-EFAULT);

	if (cache_policy == RMEM_CACHE_AGING) {
		cache_lines[slot].age = 0;
		cache_lines[slot].ref_count = 1;
		nanvix_rcache_aging(pgnum);
	} else if (cache_policy == RMEM_CACHE_NFU) {
		cache_lines[slot].age = 1;
		cache_lines[slot].ref_count = 1;
	} else {
		cache_lines[slot].age = cache_time;
	}
	return 0;
}
/*============================================================================*
 * msbDebruijn32()                                                            *
 *============================================================================*/

/**
 * @brief Search a value for its most significant bit that is setted.
 *
 * @param v Value to be used for bit search.
 *
 * @returns Upon successful completion, the index for the msb setted bit is
 * returned.
 */
uint32_t msbDeBruijn32( uint32_t v )
{
    static const int MultiplyDeBruijnBitPosition[32] =
    {
    0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
    8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
    };

    v |= v >> 1; // first round down to one less than a power of 2
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;

    return MultiplyDeBruijnBitPosition[( uint32_t )( v * 0x07C4ACDDU ) >> 27];
}

/*============================================================================*
 * random_mod()                                                               *
 *============================================================================*/

/**
 * @brief Generate a random number and it's mod by parameter value.
 *
 * @param v Value to be used for mod operation.
 *
 * @returns Upon successful completion, the mod for the random number and a
 * value is returned.
 */
int random_mod(int v)
{
    int random_number = urand();
    uint32_t msb = msbDeBruijn32(v);
    uint32_t mod_power_two = ((0x1 << msb ) - 1) & random_number;
    while(mod_power_two > (uint32_t)v)
        mod_power_two -= v;
    random_number = mod_power_two;
    return random_number;
}

/*============================================================================*
 * nanvix_rcache_fifo()                                                       *
 *============================================================================*/

/**
 * @brief Evicts pages from the cache based on the FIFO replacement policy.
 *
 * @param pgnum Number of the target page.
 *
 * @returns Upon successful completion, the index of the page is
 * returned. Upon failure a negative error code is returned instead.
 */
static int nanvix_rcache_fifo(void)
{
	int slot_idx;
	int draw_count = 1;
	AGE_TYPE age;
	AGE_TYPE min_age;

	cache_time++;

	/* Cache has space. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		if (cache_lines[i*RMEM_CACHE_BLOCK_SIZE].pgnum == RMEM_NULL)
		    return (i*RMEM_CACHE_BLOCK_SIZE);
	}

	/* No space. Make evict. */
	min_age = cache_lines[slot_idx = 0].age;
	for (int i = 1; i < RMEM_CACHE_LENGTH; i++)
	{
		age = cache_lines[i*RMEM_CACHE_BLOCK_SIZE].age;
		if (age < min_age)
		{
		    slot_idx = i*RMEM_CACHE_BLOCK_SIZE;
		    min_age = age;
		} else if (age == min_age) {
			draw_count++;
		}
	}

	if (draw_count > 1)
	{
        int random_number = random_mod(draw_count);
		int encounter_number = 0;
		for (int i = (slot_idx/RMEM_CACHE_BLOCK_SIZE); i < RMEM_CACHE_LENGTH; i++)
		{
			if ((age = cache_lines[i*RMEM_CACHE_BLOCK_SIZE].age) == min_age)
			{
				if (encounter_number == random_number)
					slot_idx = i*RMEM_CACHE_BLOCK_SIZE;
				encounter_number++;
			}
		}
	}

	for (int i = 0; i < RMEM_CACHE_BLOCK_SIZE; i++)
		if (nanvix_rcache_flush(cache_lines[slot_idx+i].pgnum) < 0)
			return (-EFAULT);

	return slot_idx;
}

/*============================================================================*
 * nanvix_rcache_nfu()                                                        *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
static int nanvix_rcache_nfu(void)
{
	return (nanvix_rcache_fifo());
}

/*============================================================================*
 * nanvix_rcache_lifo()                                                       *
 *============================================================================*/

/**
 * @brief Evicts pages from the cache based on the LIFO replacement policy.
 *
 * @param pgnum Number of the target page.
 *
 * @returns Upon successful completion, the index of the page is
 * returned. Upon failure a negative error code is returned instead.
 */
static int nanvix_rcache_lifo(void)
{
	int slot_idx;
	int age;
	int max_age;

	cache_time++;

	/* Cache has space. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		if (cache_lines[i*RMEM_CACHE_BLOCK_SIZE].pgnum == RMEM_NULL)
		    return (i*RMEM_CACHE_BLOCK_SIZE);
	}

	/* No space. Make evict. */
	max_age = cache_lines[slot_idx = 0].age;
	for (int i = 1; i < RMEM_CACHE_LENGTH; i++)
	{
		if ((age = cache_lines[i*RMEM_CACHE_BLOCK_SIZE].age) > max_age)
		{
		    slot_idx = i*RMEM_CACHE_BLOCK_SIZE;
		    max_age = age;
		}
	}

	for (int i = 0; i < RMEM_CACHE_BLOCK_SIZE; i++)
		if (nanvix_rcache_flush(cache_lines[slot_idx+i].pgnum) < 0)
			return (-EFAULT);

	return slot_idx;
}

/*============================================================================*
 * nanvix_rcache_replacement_policies()                                       *
 *============================================================================*/

/**
 * @brief Selects the replacement policy function based on the
 * replacement policy number.
 *
 * @returns Upon successful completion, the free index of a page is
 * returned. Upon failure a negative error code is returned instead.
 */
static int nanvix_rcache_replacement_policies(void)
{
	if (cache_policy == RMEM_CACHE_FIFO)
		return (nanvix_rcache_fifo());

	if (cache_policy == RMEM_CACHE_LIFO)
		return (nanvix_rcache_lifo());

	return (nanvix_rcache_nfu());
}

/*============================================================================*
 * nanvix_rcache_select_replacement_policy()                                  *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_select_replacement_policy(int num)
{
	cache_time++;

	switch (num)
	{
		case RMEM_CACHE_FIFO:
		case RMEM_CACHE_LIFO:
		case RMEM_CACHE_NFU:
		case RMEM_CACHE_AGING:
		case RMEM_CACHE_BYPASS:
			cache_policy = num;
			break;
		default:
			return (-EFAULT);
	}
	return (0);
}

/*============================================================================*
 * nanvix_rcache_select_write()                                               *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_select_write(int num)
{
	cache_time++;

	switch (num)
	{
		case RMEM_CACHE_WRITE_THROUGH:
		case RMEM_CACHE_WRITE_BACK:
			write_policy = num;
			break;
		default:
			return (-EFAULT);
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

	cache_time++;

	/* Forward allocation to remote memory. */
	if ((pgnum = nanvix_rmem_alloc()) == (rpage_t) -ENOMEM)
		return (RMEM_NULL);

	stats.nallocs++;
	return (pgnum);
}

/*============================================================================*
 * nanvix_rcache_flush()                                                      *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_flush(rpage_t pgnum)
{
	int err;

	/* Search for page in the cache. */
	struct tuple idx = nanvix_rcache_page_search(pgnum);
	int slot = idx.slot_idx;
	int block = idx.block_idx;
	int error = idx.error;

	cache_time++;

	if (error < 0)
		return (-EFAULT);

	/* Invalid page number. */
	if ((pgnum == RMEM_NULL) || (RMEM_BLOCK_NUM(pgnum) >= RMEM_NUM_BLOCKS))
		return (-EFAULT);

	/* Write page back to remote memory. */
	if ((err = nanvix_rmem_write(pgnum, cache_lines[slot+block].pages)) < 0)
		return (err);

#ifdef CACHE_DEBUG
	uprintf("[benchmark] %d misses, %d hits", stats.nmisses, stats.nhits);
#endif
	return (0);
}

/*============================================================================*
 * nanvix_rcache_free()                                                       *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_free(rpage_t pgnum)
{
	cache_time++;

	/* Invalid page number. */
	if ((pgnum == RMEM_NULL) || (RMEM_BLOCK_NUM(pgnum) >= RMEM_NUM_BLOCKS))
		return (-EFAULT);

	/* Check if target page is loaded into the cache. */
	for (int i = 0; i < RMEM_CACHE_LENGTH; i++)
	{
		if (cache_lines[i].pgnum == pgnum)
		    cache_lines[i].pgnum = RMEM_NULL;
	}

	stats.nallocs--;
	return (nanvix_rmem_free(pgnum));
}

/*============================================================================*
 * nanvix_rcache_get()                                                        *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
void *nanvix_rcache_get(rpage_t pgnum)
{
	int err;
	int evict_idx;
	void *ptr;

	struct tuple idx = nanvix_rcache_page_search(pgnum);
	int slot = idx.slot_idx;
	int block = idx.block_idx;
	int error = idx.error;

	cache_time++;

	/* Invalid page number. */
	if ((pgnum == RMEM_NULL) || (RMEM_BLOCK_NUM(pgnum) >= RMEM_NUM_BLOCKS))
		return (NULL);

	/* Normal mode. */
	if (cache_policy != RMEM_CACHE_BYPASS)
	{
		if (error >= 0)
		{
			stats.nhits++;
			nanvix_rcache_age_update(pgnum);
			cache_lines[slot].ref_count++;
			return (cache_lines[slot+block].pages);
		}

		stats.nmisses++;
		if ((evict_idx = nanvix_rcache_replacement_policies()) < 0)
			return (NULL);
		/* Load page remote page. */
		for (int i = 0; i < RMEM_CACHE_BLOCK_SIZE; i++)
		{
			if ((err = nanvix_rmem_read((rpage_t)(pgnum+i), cache_lines[evict_idx+i].pages)) < 0)
				return (NULL);
			cache_lines[evict_idx+i].pgnum = (rpage_t)(pgnum+i);
		}

		cache_lines[evict_idx].ref_count++;
		nanvix_rcache_age_init(pgnum);

		ptr = cache_lines[evict_idx].pages;
	}
	/* Bypass mode. */
	else
	{
		stats.nmisses++;

		if (cache_lines[0].pgnum != RMEM_NULL)
		{
			if (nanvix_rcache_flush(cache_lines[0].pgnum))
				return (NULL);
		}

		if ((err = nanvix_rmem_read(pgnum, cache_lines[0].pages)) < 0)
			return (NULL);

		cache_lines[0].pgnum = pgnum;
		ptr = cache_lines[0].pages;
	}

#ifdef CACHE_DEBUG
	uprintf("[benchmark] %d misses, %d hits", stats.nmisses, stats.nhits);
#endif

	return (ptr);
}

/*============================================================================*
 * nanvix_rcache_put()                                                        *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_rcache_put(rpage_t pgnum, int strike)
{
	struct tuple idx = nanvix_rcache_page_search(pgnum);
	int slot = idx.slot_idx;
	int error = idx.error;

	cache_time++;

	if (error < 0)
		return (-EFAULT);

	/* Invalid page number. */
	if ((pgnum == RMEM_NULL) || (RMEM_BLOCK_NUM(pgnum) >= RMEM_NUM_BLOCKS))
		return (-EFAULT);

	if (cache_policy != RMEM_CACHE_BYPASS)
	{

#ifndef USE_STRIKES
		strike = 0;
#endif

		if (cache_policy == RMEM_CACHE_NFU)
			cache_lines[slot].age += strike;

		if (cache_lines[slot].ref_count <= 0)
			return (-EFAULT);

		if ((write_policy == RMEM_CACHE_WRITE_THROUGH) && (nanvix_rcache_flush(pgnum) < 0))
			return (-EFAULT);

		cache_lines[slot].ref_count--;
	}
	else
	{
		int err;

		if (cache_lines[0].pgnum != pgnum)
			return (-EFAULT);

		if ((err = nanvix_rmem_write(pgnum, cache_lines[0].pages)) < 0)
			return (err);
	}

#ifdef CACHE_DEBUG
	uprintf("[benchmark] %d misses, %d hits", stats.nmisses, stats.nhits);
#endif
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
	/* Page cache already initialized. */
	if (!initialized)
		return (0);

	/* Initialize page cache statistics. */
	stats.nmisses = 0;
	stats.nhits = 0;
	stats.nallocs = 0;

	/* Page cache lines. */
	for (int i = 0; i < RMEM_CACHE_SIZE; i++)
	{
		cache_lines[i].pgnum = RMEM_NULL;
		cache_lines[i].age = 0;
		cache_lines[i].ref_count = 0;
	}

	initialized = 1;

	return (0);
}
