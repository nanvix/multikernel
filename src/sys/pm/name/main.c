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

#define __NAME_SERVICE

#include <nanvix/servers/message.h>
#include <nanvix/servers/name.h>
#include <nanvix/servers/spawn.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/semaphore.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/stdint.h>

#define __DEBUG_NAME 0

#if (__DEBUG_NAME)
	static char debug_str[64];
	#define name_debug(fmt, ...) uprintf(fmt, __VA_ARGS__)
#else
	#define name_debug(fmt, ...) { }
#endif

/**
 * @brief Number of registration.
 */
static int nr_registration = 0;

/**
 * @brief Inbox.
 */
static int inbox = -1;

/**
 * @brief Lookup table of process names.
 */
static struct {
	int nodenum;                     /**< NoC node.                 */
	char name[NANVIX_PROC_NAME_MAX]; /**< Process name.             */
	uint64_t timestamp;              /**< Timestamp for heartbeats. */
} procs[NANVIX_PROC_MAX];

/**
 * @brief Server stats.
 */
static struct
{
	int nlinks;         /**< Number of name link requests.   */
	int nunlinks;       /**< Number of unlink name requests. */
	int nlookups;       /**< Number of lookup requests.      */
} stats = { 0, 0, 0};

/*===================================================================*
 * do_name_init()                                                    *
 *===================================================================*/

/**
 * @brief Initializes the name server.
 */
static void do_name_init(struct nanvix_semaphore *lock)
{
	/* Initialize lookup table. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		procs[i].nodenum = -1;
		procs[i].timestamp = 0;
	}

	procs[0].nodenum = knode_get_num();
	ustrcpy(procs[0].name, "/io0");

	uassert((inbox = stdinbox_get()) >= 0);

	/* Unblock spawner. */
	uprintf("[nanvix][name] server alive");
	uprintf("[nanvix][name] listening to mailbox %d", inbox);
	uprintf("[nanvix][name] syncing in sync %d", stdsync_get());
	uprintf("[nanvix][name] attached to node %d", knode_get_num());

	nanvix_semaphore_up(lock);
}

/*=======================================================================*
 * do_name_lookup()                                                      *
 *=======================================================================*/

/**
 * @brief Converts a name into a NoC node number.
 *
 * @param requet   Request.
 * @param response Response.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
static int do_name_lookup(
	const struct name_message *request,
	struct name_message *response
)
{
	int ret;
	const char *name;

	name = request->op.lookup.name;
	response->op.ret.nodenum = -1;

	stats.nlookups++;
	name_debug("lookup name=%s", name);

	/* Invalid name. */
	if ((ret = name_is_valid(name)) < 0)
		return (ret);

	/* Search for portal name. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		/* Found. */
		if (!ustrcmp(name, procs[i].name))
		{
			response->op.ret.nodenum = procs[i].nodenum;
			return (0);
		}
	}

	return (-ENOENT);
}

/*=======================================================================*
 * do_name_link()                                                        *
 *=======================================================================*/

/**
 * @brief Register a process name.
 *
 * @param requet Request.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_name_link(const struct name_message *request)
{
	int ret;
	int index;
	int nodenum;
	const char *name;

	name = request->op.link.name;
	nodenum = request->header.source;

	stats.nlinks++;
	name_debug("link nodenum=%d name=%s", nodenum, name);

	/* Invalid node number. */
	if (!proc_is_valid(nodenum))
		return (-EINVAL);

	/* Invalid name. */
	if ((ret = name_is_valid(name)) < 0)
		return (ret);

	/* No entry available. */
	if (nr_registration >= NANVIX_PROC_MAX)
		return (-EINVAL);

	/* Check that the name is not already used */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		if (ustrcmp(procs[i].name, name) == 0)
			return (-EINVAL);
	}

	/* Find index. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		/* Found. */
		if (procs[i].nodenum == -1)
		{
			index = i;
			goto found;
		}
	}

	return (-EINVAL);

found:

	nr_registration++;
	procs[index].nodenum = nodenum;
	ustrcpy(procs[index].name, name);

	return (0);
}

/*=======================================================================*
 * do_name_unlink()                                                      *
 *=======================================================================*/

/**
 * @brief Remove a name
 *
 * @param requet Request.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
static int do_name_unlink(const struct name_message *request)
{
	int ret;
	const char *name;

	name = request->op.unlink.name;

	stats.nlinks++;
	name_debug("unlink name=%s", name);

	/* Invalid name. */
	if ((ret = name_is_valid(name)) < 0)
		return (ret);

	/* Search for name */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		/* Skip invalid entries. */
		if (procs[i].nodenum == -1)
			continue;

		/* Found*/
		if (ustrcmp(procs[i].name, name) == 0)
		{
			nr_registration--;
			ustrcpy(procs[i].name, "");
			procs[i].nodenum = -1;
			return (0);
		}
	}

	return (-ENOENT);
}

/*=======================================================================*
 * do_name_heartbeat()                                                   *
 *=======================================================================*/

/**
 * @brief Updates the heartbeat of a process.
 *
 * @param request,
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
static int do_name_heartbeat(const struct name_message *request)
{
	int nodenum;
	uint64_t timestamp;

	timestamp = request->op.heartbeat.timestamp;
	nodenum = request->header.source;

	name_debug("heartbeat nodenum=%d timestap=%l", nodenum, timestamp);

	/* Invalid node number. */
	if (!proc_is_valid(nodenum))
		return (-EINVAL);

	/* Record timestamp. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		if (procs[i].nodenum == nodenum)
		{
			procs[i].timestamp = timestamp;
			return (0);
		}
	}

	return (-EINVAL);
}

/*===================================================================*
 * name_server()                                                     *
 *===================================================================*/

/**
 * @brief Handles remote name requests.
 *
 * @returns Always returns 0.
 */
int do_name_server(struct nanvix_semaphore *lock)
{
	int shutdown = 0;

	uprintf("[nanvix][name] booting up server");
	do_name_init(lock);

	while (!shutdown)
	{
		int outbox;
		int reply = 0;
		int ret = -ENOSYS;
		struct name_message request;
		struct name_message response;

		uassert(
			kmailbox_read(
				inbox,
				&request,
				sizeof(struct name_message)
			) == sizeof(struct name_message)
		);

		#if (__DEBUG_NAME)
		message_header_sprint(debug_str, &request.header);
		uprintf("name request %s", debug_str);
		#endif

		/* Handle name requests. */
		switch (request.header.opcode)
		{
			/* Lookup. */
			case NAME_LOOKUP:
				ret = do_name_lookup(&request, &response);
				reply = 1;
				break;

			/* Add name. */
			case NAME_LINK:
				ret =  do_name_link(&request);
				reply = 1;
				break;

			/* Remove name. */
			case NAME_UNLINK:
				stats.nunlinks++;
				ret = do_name_unlink(&request);
				reply = 1;
				break;

			case NAME_ALIVE:
				uassert((ret = do_name_heartbeat(&request)) == 0);
				break;

			case NAME_EXIT:
				shutdown = 1;
				break;

			/* Should not happen. */
			default:
				break;
		}

		/* No reply? */
		if (!reply)
			continue;

		response.op.ret.errcode = ret;
		message_header_build(
			&response.header,
			(ret <= 0) ? NAME_FAIL : NAME_SUCCESS
		);

		uassert((
			outbox = kmailbox_open(
				request.header.source,
				request.header.mailbox_port
			)) >= 0
		);
		uassert(
			kmailbox_write(
				outbox,
				&response,
				sizeof(struct name_message
			)) == sizeof(struct name_message)
		);
		uassert(kmailbox_close(outbox) == 0);
	}

	/* Dump statistics. */
	uprintf("[nanvix][name] links=%d lookups=%d unlinks=%d",
			stats.nlinks, stats.nlookups, stats.nunlinks
	);

	return (0);
}

/*============================================================================*
 * __main2()                                                                  *
 *============================================================================*/

/**
 * @brief Handles remote name requests.
 *
 * @returns Always returns zero.
 */
int name_server(struct nanvix_semaphore *lock)
{
	do_name_server(lock);

	return (0);
}
