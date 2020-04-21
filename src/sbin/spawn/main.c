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

#define SPAWN_SERVER

#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/servers/spawn.h>
#include <nanvix/sys/semaphore.h>
#include <nanvix/sys/noc.h>
#include <nanvix/sys/thread.h>
#include <nanvix/ulib.h>

/* Import definitions. */
extern const int SERVERS_NUM;
extern const struct serverinfo *SERVERS;
extern const char *spawner_name;

/**
 * @brief Startup lock.
 */
static struct nanvix_semaphore lock;

/*============================================================================*
 * Server Wrapper                                                             *
 *============================================================================*/

/**
 * @brief Server wrapper.
 */
static void *server(void *args)
{
	int servernum;
	int (*main_fn) (struct nanvix_semaphore *);

	servernum = ((int *)args)[0];
	main_fn = SERVERS[servernum].main;

		if (SERVERS[servernum].ring != SPAWN_RING_X)
			__runtime_setup(SERVERS[servernum].ring);

		main_fn(&lock);

		if (SERVERS[servernum].ring != SPAWN_RING_X)
			__runtime_cleanup();

	return (NULL);
}

/*============================================================================*
 * __main2()                                                                  *
 *============================================================================*/

/**
 * @brief Spawns servers and user processes.
 *
 * @param argc Argument count (unused).
 * @param argv Argument list (unused).
 *
 * @returns Always returns zero.
 */
int __main2(int argc, const char *argv[])
{
	int args[THREAD_MAX];
	kthread_t tids[THREAD_MAX];

	((void) argc);
	((void) argv);

	uassert(SERVERS_NUM <= (THREAD_MAX - 1));

	__runtime_setup(SPAWN_RING_0);

		nanvix_semaphore_init(&lock, 0);

		uprintf("[nanvix][%s] attached to node %d", spawner_name, knode_get_num());
		uprintf("[nanvix][%s] listening to inbox %d", spawner_name, stdinbox_get());
		uprintf("[nanvix][%s] syncing in sync %d", spawner_name, stdsync_get());

		uassert(stdsync_fence() == 0);
		spawn_barrier_setup();

		/* Spawn servers. */
		for (int ring = SPAWN_RING_FIRST; ring <= SPAWN_RING_LAST; ring++)
		{
			uprintf("[nanvix][%s] spawning servers in ring %d...", spawner_name, ring);
			for (int i = 0; i < SERVERS_NUM; i++)
			{
				if (SERVERS[i].ring == ring)
				{
					args[i] = i;
					uassert(kthread_create(&tids[i], server, &args[i]) == 0);
					nanvix_semaphore_down(&lock);
				}
			}

			spawn_barrier_wait();
		}

		uassert(stdsync_fence() == 0);
		uassert(stdsync_fence() == 0);
		uprintf("[nanvix][%s] waiting shutdown signal...", spawner_name);

		/* Wait for servers. */
		for (int i = 0; i < SERVERS_NUM; i++)
		{
			uassert(kthread_join(tids[i], NULL) == 0);
			uprintf("[nanvix][%s] server %d down...", spawner_name, i);
		}

		spawn_barrier_wait();
		spawn_barrier_cleanup();

		uprintf("[nanvix][%s] shutting down...", spawner_name);

	__runtime_cleanup();

	return (0);
}
