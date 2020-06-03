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

#define __NEED_SPAWN_SERVER

#include <nanvix/servers/spawn.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>

/**
 * @brief Startup barrier
 */
static struct
{
	int syncs[2];
} barrier;

#ifndef __unix64__

/**
 * @brief Forces a platform-independent delay.
 *
 * @param cycles Delay in cycles.
 *
 * @author João Vicente Souto
 */
static void barrier_delay(int times, uint64_t cycles)
{
	uint64_t t0, t1;

	for (int i = 0; i < times; ++i)
	{
		kclock(&t0);

		do
			kclock(&t1);
		while ((t1 - t0) < cycles);
	}
}

#endif /* !__unix64__ */

/**
 * @brief Initializes the spawn barrier.
 */
void spawn_barrier_setup(void)
{
	int nodes[SPAWNERS_NUM] = {
	#if defined(__mppa256__)
		SPAWN_SERVER_0_NODE,
		SPAWN_SERVER_1_NODE
	#elif defined (__unix64__)
		SPAWN_SERVER_0_NODE,
		SPAWN_SERVER_1_NODE,
		SPAWN_SERVER_2_NODE,
		SPAWN_SERVER_3_NODE
	#endif
	};

	/* Leader. */
	if (kcluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
	{
		uassert((
			barrier.syncs[0] = ksync_create(
				nodes,
				SPAWNERS_NUM,
				SYNC_ALL_TO_ONE)
			) >= 0
		);
		uassert((
			barrier.syncs[1] = ksync_open(
				nodes,
				SPAWNERS_NUM,
				SYNC_ONE_TO_ALL)
			) >= 0
		);
	}

	/* Follower. */
	else
	{
		uassert((
			barrier.syncs[1] = ksync_create(
				nodes,
				SPAWNERS_NUM,
				SYNC_ONE_TO_ALL)
			) >= 0
		);
		uassert((
			barrier.syncs[0] = ksync_open(
				nodes,
				SPAWNERS_NUM,
				SYNC_ALL_TO_ONE)
			) >= 0
		);

#ifndef __unix64__
		barrier_delay(1, CLUSTER_FREQ);
#endif /* !__unix64__ */
	}
}

/**
 * @brief Shutdowns the spawn barrier.
 */
void spawn_barrier_cleanup(void)
{
	/* Leader. */
	if (kcluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
	{
		uassert(ksync_unlink(barrier.syncs[0]) == 0);
		uassert(ksync_close(barrier.syncs[1]) == 0);
	}

	/* Follower. */
	else
	{
		uassert(ksync_close(barrier.syncs[0]) == 0);
		uassert(ksync_unlink(barrier.syncs[1]) == 0);
	}
}

/**
 * @brief Waits on the startup barrier.
 */
void spawn_barrier_wait(void)
{
	/* Leader */
	if (kcluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER)
	{
		uassert(ksync_wait(barrier.syncs[0]) == 0);
		uassert(ksync_signal(barrier.syncs[1]) == 0);
	}

	/* Follower. */
	else
	{
		uassert(ksync_signal(barrier.syncs[0]) == 0);
		uassert(ksync_wait(barrier.syncs[1]) == 0);
	}
}
