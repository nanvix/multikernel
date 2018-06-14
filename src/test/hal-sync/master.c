/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#include <mppa/osconfig.h>

#include <nanvix/config.h>
#include <nanvix/hal.h>

/**
 * @brief Number of cores in the underlying cluster.
 */
static int ncores = 0;

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Global barrier for synchronization.
 */
static pthread_barrier_t barrier;

/**
 * @brief Lock for critical sections.
 */
static pthread_mutex_t lock;

/*===================================================================*
 * API Test: Create Unlink                                           *
 *===================================================================*/

/**
 * @brief API Test: Synchronization Point Create Unlink
 */
static void *test_hal_sync_thread_create_unlink(void *args)
{
	int syncid;
	int *nodes;

	hal_setup();

	nodes = ((int *)args);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Create Unlink
 */
static void test_hal_sync_create_unlink(void)
{
	int nodes[ncores];
	pthread_t tids[ncores];

	printf("[test][api] Sync Create Unlink\n");

	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_sync_thread_create_unlink,
			nodes)) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * API Test: Open Close                                              *
 *===================================================================*/

/**
 * @brief API Test: Synchronization Point Open Close
 */
static void *test_hal_sync_thread_open_close(void *args)
{
	int syncid;
	int *nodes;

	hal_setup();

	nodes = ((int *)args);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Open Close
 */
static void test_hal_sync_master_open_close(const int *nodes)
{
	int syncid;

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_open(nodes, ncores)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
	pthread_mutex_unlock(&lock);
}

/**
 * @brief API Test: Synchronization Point Open Close
 */
static void test_hal_sync_open_close(void)
{
	int nodes[ncores];
	pthread_t tids[ncores];

	printf("[test][api] Sync Open Close\n");

	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_sync_thread_open_close,
			nodes)) == 0
		);
	}

	test_hal_sync_master_open_close(nodes);

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * API Test: Wait Signal                                             *
 *===================================================================*/

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void *test_hal_sync_thread_wait_signal(void *args)
{
	int syncid;
	int *nodes;

	hal_setup();

	nodes = ((int *)args);

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_create(nodes, ncores, HAL_SYNC_ONE_TO_ALL)) >= 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	TEST_ASSERT(hal_sync_wait(syncid) == 0);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_unlink(syncid) == 0);
	pthread_mutex_unlock(&lock);

	pthread_barrier_wait(&barrier);

	hal_cleanup();
	return (NULL);
}

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void test_hal_sync_master_wait_signal(const int *nodes)
{
	int syncid;

	pthread_mutex_lock(&lock);
	TEST_ASSERT((syncid = hal_sync_open(nodes, ncores)) >= 0);
	pthread_mutex_unlock(&lock);

	TEST_ASSERT(hal_sync_signal(syncid, HAL_SYNC_ONE_TO_ALL) == 0);

	pthread_mutex_lock(&lock);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
	pthread_mutex_unlock(&lock);
}

/**
 * @brief API Test: Synchronization Point Wait Signal
 */
static void test_hal_sync_wait_signal(void)
{
	int nodes[ncores];
	pthread_t tids[ncores];

	printf("[test][api] Sync Wait Signal\n");

	/* Build nodes list. */
	for (int i = 0; i < ncores; i++)
		nodes[i] = hal_get_node_id() + i;

	/* Spawn driver threads. */
	for (int i = 1; i < ncores; i++)
	{
		assert((pthread_create(&tids[i],
			NULL,
			test_hal_sync_thread_wait_signal,
			nodes)) == 0
		);
	}

	test_hal_sync_master_wait_signal(nodes);

	/* Wait for driver threads. */
	for (int i = 1; i < ncores; i++)
		pthread_join(tids[i], NULL);
}

/*===================================================================*
 * Synchronization Point Test Driver                                 *
 *===================================================================*/

/**
 * @brief Synchronization Point Test Driver
 */
int main(int argc, const char **argv)
{
	((void) argc);
	((void) argv);

	hal_setup();

	ncores = hal_get_num_cores();

	pthread_mutex_init(&lock, NULL);
	pthread_barrier_init(&barrier, NULL, ncores - 1);

	/* API tests. */
	test_hal_sync_create_unlink();
	test_hal_sync_open_close();
	test_hal_sync_wait_signal();

	hal_cleanup();
	return (EXIT_SUCCESS);
}
