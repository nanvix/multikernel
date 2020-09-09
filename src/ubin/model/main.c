#define __NEED_MM_RCACHE

#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/mm.h>
#include <nanvix/ulib.h>
#include <nanvix/pm.h>
#include <posix/stdint.h>
#include "benchmark.h"
#include "fn_sequential_x86_42pages_21x_20y_10000red.h"

/**
 * @brief Ulibc is missing this.
 */
#define URAND_MAX (127773*16807 + 2836)

/**
 * @brief Define which struct to use.
 */
#define APPS_STRUCT fn_sequential_x86_42pages_21x_20y_10000red

/**
 * @brief Number of trials.
 */
#ifndef __NTRIALS
#define NTRIALS 50
#endif

/**
 * @brief Number of pages.
 */
#ifndef __NUM_PAGES
#define NUM_PAGES (RMEM_SERVERS_NUM*(RMEM_NUM_BLOCKS - 1))
#endif

/**
 * @brief Indexer for 2D arrays.
 */
#define ARRAY2D(a,w,i,j) (a)[(i)*(w) + (j)]

struct division
{
	int quotient;
	int remainder;
};

struct division divide(int a, int b)
{
	struct division result;

	result.quotient = 0;
	result.remainder = a;

	while (result.remainder >= b)
	{
		result.remainder -= b;
		result.quotient++;
	}

	return (result);
}

static rpage_t raw_pages[NUM_PAGES];

/**
 * @brief Syntetic Benchmark
 */
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	int rand_number;
	int page_value = 0;
	int sum = 0;
	int selection = -1;
    int skipped = 0;
    int access_time = 1;
	int total_ocurrences;
	int row_size;
	int column_size;
	int random_num = 0;
	int trials;
	int total_trials = 0;
	struct division app_rand;
	struct division roulette_rand;
	usrand(9876);

	__runtime_setup(SPAWN_RING_FIRST);

		/* Unblock spawners. */
		uassert(stdsync_fence() == 0);
		uprintf("[nanvix][benchmark] server starting...");
		uassert(stdsync_fence() == 0);
		uassert(stdsync_fence() == 0);
		uprintf("[nanvix][benchmark] server alive");

	__runtime_setup(SPAWN_RING_LAST);

		/* Allocate pages. */
		uprintf("[benchmark] allocating pages: %d\n", NUM_PAGES);
		for (int i = 0; i < NUM_PAGES; i++)
			uassert((raw_pages[i] = nanvix_rcache_alloc()) != 0);

		column_size = APPS_STRUCT.col[0];
		for (int j = 0; j < column_size; j++)
			total_trials += APPS_STRUCT.trials[0][j];
		uprintf("[benchmark] total trials: %d\n", total_trials);

		/* Run matrix. */
		uprintf("[benchmark] applying puts and gets\n");
		for (int j = 0; j < column_size; j++)
		{
			total_ocurrences = 0;
			trials = APPS_STRUCT.trials[0][j];


			/* Run trials on column. */
			for (int trial = 0; trial < trials; trial++, access_time++)
			{
				/* Num for a specific app */
				random_num = 0;
				app_rand = divide(urand(), APPS_STRUCT.size);
				random_num = app_rand.remainder;
				/* random_num =  random_mod(APPS_STRUCT.size); */

				/* Computer number of occurences in a column. */
				row_size = APPS_STRUCT.row[random_num];
				for (int i = 0; i < row_size; i++)
				{
					total_ocurrences += ARRAY2D(APPS_STRUCT.work[random_num], column_size, i, j);
				}

				/* This will be commented but it is better for unix64.*/
				roulette_rand = divide(urand(), total_ocurrences);
				rand_number = roulette_rand.remainder;

				/* rand_number = random_mod(total_ocurrences); */
				/* Probability Roulette */
				sum = 0;
				selection = -1;
				for (int i = 0; i < row_size; i++)
				{
					sum += ARRAY2D(APPS_STRUCT.work[random_num], column_size, i, j);

					/* If first elements of column are equal to zero. */
					if (rand_number == 0 && sum <= 0)
						continue;

					/* Roulette core. */
					if ((rand_number - sum) <= 0)
					{
						selection = i;
						break;
					}
				}
				/* If all column elements are equal to zero, skip. */
				if (selection == -1)
					continue;

				for (int i = 0; i <= random_num-1; i++)
					page_value += APPS_STRUCT.row[i]-1;
				page_value += APPS_STRUCT.pages_interval[random_num][selection].high;

				uprintf("[benchmark][heatmap] %d %d\n", access_time, page_value);
                uprintf("[benchmark] iteration %d of %d\n", access_time, total_trials);

				uprintf("%d\n", page_value);
				uassert(nanvix_rcache_get(raw_pages[page_value]) != NULL);
				uassert(nanvix_rcache_put(raw_pages[page_value], 0) == 0);
				uprintf("[benchmark] Access %d\n", j);
				total_ocurrences = 0;
				page_value = 0;
			}
		}
		uprintf("[benchmark] %d lines skipped\n", skipped);

		/* Free pages. */
		uprintf("[benchmark] freeing pages: %d\n", NUM_PAGES);
		for (int i = 0; i < NUM_PAGES; i++)
			uassert(nanvix_rcache_free(raw_pages[i]) == 0);

		uprintf("[nanvix][test] shutting down server");
		uassert(stdsync_fence() == 0);

	nanvix_shutdown();


	__runtime_cleanup();

	return (0);
}
