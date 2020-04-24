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

#include <nanvix/servers/message.h>
#include <nanvix/servers/spawn.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/ulib.h>

/**
 * @brief Name Server message.
 */
struct spawn_message
{
	message_header header;
};

/**
 * @brief Port number for Spawn Server.
 */
#define SPAWN_SERVER_PORT_NUM 1

/**
 * @brief Startup barrier
 */
static struct
{
	int mailboxes[SPAWNERS_NUM];
} barrier = {
	.mailboxes = {
		[0 ... (SPAWNERS_NUM - 1)] = -1
	}
};

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
		for (int i = 1 ; i < SPAWNERS_NUM; i++)
		{
			uassert((barrier.mailboxes[i] = kmailbox_open(
				nodes[i], SPAWN_SERVER_PORT_NUM)
			) >= 0);
		}
	}

	/* Follower. */
	else
	{
		uassert((barrier.mailboxes[0] = kmailbox_open(
			nodes[0], SPAWN_SERVER_PORT_NUM)
		) >= 0);
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
		for (int i = 1 ; i < SPAWNERS_NUM; i++)
			uassert(kmailbox_close(barrier.mailboxes[i]) == 0);
	}

	/* Follower. */
	else
		uassert(kmailbox_close(barrier.mailboxes[0]) == 0);
}

/**
 * @brief Waits on the startup barrier
 */
void spawn_barrier_wait(void)
{
	struct spawn_message msg;

	/* Leader */
	if (cluster_get_num() == SPAWN_SERVER_0_NODE)
	{
		for (int i = 1 ; i < SPAWNERS_NUM; i++)
		{
			uassert(kmailbox_read(
				stdinbox_get(), &msg, sizeof(struct spawn_message)
			) == sizeof(struct spawn_message));
		}
		for (int i = 1 ; i < SPAWNERS_NUM; i++)
		{
			uassert(kmailbox_write(
				barrier.mailboxes[i], &msg, sizeof(struct spawn_message)
			) == sizeof(struct spawn_message));
		}
	}

	/* Follower. */
	else
	{
		uassert(kmailbox_write(
			barrier.mailboxes[0], &msg, sizeof(struct spawn_message)
		) == sizeof(struct spawn_message));
		uassert(kmailbox_read(
			stdinbox_get(), &msg, sizeof(struct spawn_message)
		) == sizeof(struct spawn_message));
	}

}
