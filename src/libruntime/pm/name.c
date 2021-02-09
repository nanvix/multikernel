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
#define __NEED_NAME_SERVER
#define __NEED_NAME_SERVICE

#include <nanvix/kernel/mailbox.h>
#include <nanvix/sys/condvar.h>
#include <nanvix/sys/mutex.h>
#include <nanvix/sys/thread.h>
#include <nanvix/servers/name.h>
#include <nanvix/runtime/pm.h>
#include <nanvix/config.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/stdbool.h>

#define DEBUG 0

/**
 * @brief ID of name snooper thread.
 */
static kthread_t nanvix_name_snooper_tid = -1;

/**
 * @brief Mailbox for small messages.
 */
static int server;

/**
 * @brief Is the name service initialized ?
 */
static bool initialized = false;

/**
 * @brief Local resolution table.
 */
PRIVATE struct {
	char name[NANVIX_PROC_NAME_MAX]; /**< Process name.          */
	int inbox_port_nr;               /**< Default inbox port nr. */
} _local_names[NANVIX_LOCAL_PNAME_MAX] = {
	[0 ... (NANVIX_LOCAL_PNAME_MAX - 1)] = {
		.name = "",
		.inbox_port_nr = -1
	}
};

PRIVATE struct nanvix_mutex _name_lock;

/**
 * @brief Locking variables for address lookups.
 */
PRIVATE struct nanvix_mutex _local_lock;

PRIVATE struct nanvix_cond_var _local_condvar;

PRIVATE struct
{
	char name[NANVIX_PROC_NAME_MAX];
	int port_nr;
	int read;
} _addr_lookup_result = {
	.name    = "",
	.port_nr = -1,
	.read    =  1
};

/*============================================================================*
 * LOCAL OPERATIONS                                                           *
 *============================================================================*/

/**
 * @brief Name resolution local snooper.
 *
 * @param name Process name to be searched for.
 *
 * @returns Upon successful completion, the port associated with @p name is
 * returned if it was already registered. Upon an unregistered name, a negative
 * number is returned instead.
 */
PRIVATE int _local_address_lookup(const char *name)
{
	/* Traverses the local resolution table searching for @p name. */
	for (int i = 0; i < NANVIX_LOCAL_PNAME_MAX; ++i)
	{
		/* Jump through unused indexes. */
		if (_local_names[i].inbox_port_nr == (-1))
			continue;

		/* Found? */
		if (ustrcmp(_local_names[i].name, name) == 0)
			return (_local_names[i].inbox_port_nr);
	}

	/* No processes were registered with @p name. */
	return (-1);
}

/**
 * @brief Registers a process name in the local resolution table.
 *
 * @param inbox_port Port number of the default inbox associated.
 * @param name       Target process name.
 *
 * @returns Upon successful completion, zero is returned. A negative error
 * code is returned instead.
 */
PRIVATE int _local_name_register(int inbox_port, const char *name)
{
	/* Searches for a free index in the local resolution table. */
	for (int i = 0; i < NANVIX_LOCAL_PNAME_MAX; ++i)
	{
		/* Found? */
		if (_local_names[i].inbox_port_nr != (-1))
			continue;

		/* Registers the process info in the found index. */
		ustrcpy(_local_names[i].name, name);
		_local_names[i].inbox_port_nr = inbox_port;

		return (0);
	}

	return (-1);
}

/**
 * @brief Unregisters a process name in the local resolution table.
 *
 * @param name Process name to be searched for.
 */
PRIVATE void _local_name_unregister(const char *name)
{
	/* Searches for a free index in the local resolution table. */
	for (int i = 0; i < NANVIX_LOCAL_PNAME_MAX; ++i)
	{
		/* Jump through unused indexes. */
		if (_local_names[i].inbox_port_nr == (-1))
			continue;

		/* Found? */
		if (ustrcmp(_local_names[i].name, name) != 0)
			continue;

		/* Removes the process info of the resolution table. */
		ustrcpy(_local_names[i].name, "");
		_local_names[i].inbox_port_nr = -1;

		break;
	}
}

/*============================================================================*
 * REMOTE OPERATIONS                                                          *
 *============================================================================*/

/*============================================================================*
 * nanvix_name_lookup()                                                       *
 *============================================================================*/

/**
 * @todo TODO: provide a long description for this function.
 */
int nanvix_name_lookup(const char *name)
{
	int ret;
	int inbox;
	struct name_message msg;

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_name_is_valid(name)) < 0)
		return (ret);

	/* Build operation header. */
	message_header_build(&msg.header, NAME_LOOKUP);
	ustrcpy(msg.op.lookup.name, name);

	uassert(nanvix_mutex_lock(&_name_lock) == 0);

	if ((ret = kmailbox_write(server, &msg, sizeof(struct name_message))) != sizeof(struct name_message))
	{
		uassert(nanvix_mutex_unlock(&_name_lock) == 0);
		return (ret);
	}

	inbox = stdinbox_get();

	/**
	 * @brief Sets the stdinbox remote to receive exclusively from the Name Server outbox.
	 *
	 * @todo Get further information about the necessity of a stronger message selection.
	 */
	uassert(kmailbox_set_remote(inbox, NAME_SERVER_NODE, MAILBOX_ANY_PORT) == 0);

	if ((ret = kmailbox_read(inbox, &msg, sizeof(struct name_message))) != sizeof(struct name_message))
	{
		uassert(nanvix_mutex_unlock(&_name_lock) == 0);
		return (ret);
	}

	uassert(nanvix_mutex_unlock(&_name_lock) == 0);

	return (msg.op.ret.nodenum);
}

/*============================================================================*
 * nanvix_name_link()                                                         *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_name_link(int nodenum, const char *name)
{
	int ret;
	int inbox;
	struct name_message msg;

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Invalid NoC node ID. */
	if (!proc_is_valid(nodenum))
		return (-EINVAL);

	/* Invalid name. */
	if ((ret = nanvix_name_is_valid(name)) < 0)
		return (ret);

	/* Build operation header. */
	message_header_build(&msg.header, NAME_LINK);
	ustrcpy(msg.op.link.name, name);

	uassert(nanvix_mutex_lock(&_name_lock) == 0);

	if ((ret = kmailbox_write(server, &msg, sizeof(struct name_message))) != sizeof(struct name_message))
	{
		uassert(nanvix_mutex_unlock(&_name_lock) == 0);
		return (ret);
	}

	inbox = stdinbox_get();

	/**
	 * @brief Sets the stdinbox remote to receive exclusively from the Name Server outbox.
	 *
	 * @todo Get further information about the necessity of a stronger message selection.
	 */
	uassert(kmailbox_set_remote(inbox, NAME_SERVER_NODE, MAILBOX_ANY_PORT) == 0);

	if ((ret = kmailbox_read(inbox, &msg, sizeof(struct name_message))) != sizeof(struct name_message))
	{
		uassert(nanvix_mutex_unlock(&_name_lock) == 0);
		return (ret);
	}

	uassert(nanvix_mutex_unlock(&_name_lock) == 0);

	if (msg.header.opcode == NAME_SUCCESS)
		return (0);

	return (msg.op.ret.errcode);
}

/*============================================================================*
 * nanvix_name_unlink()                                                       *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_name_unlink(const char *name)
{
	int ret;
	int inbox;
	struct name_message msg;

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_name_is_valid(name)) < 0)
		return (ret);

	/* Build operation header. */
	message_header_build(&msg.header, NAME_UNLINK);
	ustrcpy(msg.op.unlink.name, name);

	uassert(nanvix_mutex_lock(&_name_lock) == 0);

	if ((ret = kmailbox_write(server, &msg, sizeof(struct name_message))) != sizeof(struct name_message))
	{
		uassert(nanvix_mutex_unlock(&_name_lock) == 0);
		return (ret);
	}

	inbox = stdinbox_get();

	/**
	 * @brief Sets the stdinbox remote to receive exclusively from the Name Server outbox.
	 *
	 * @todo Get further information about the necessity of a stronger message selection.
	 */
	uassert(kmailbox_set_remote(inbox, NAME_SERVER_NODE, MAILBOX_ANY_PORT) == 0);

	if ((ret = kmailbox_read(inbox, &msg, sizeof(struct name_message))) != sizeof(struct name_message))
	{
		uassert(nanvix_mutex_unlock(&_name_lock) == 0);
		return (ret);
	}

	uassert(nanvix_mutex_unlock(&_name_lock) == 0);

	if (msg.header.opcode == NAME_SUCCESS)
		return (0);

	return (msg.op.ret.errcode);
}

/*============================================================================*
 * nanvix_name_heartbeat()                                                    *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_name_heartbeat(void)
{
	int ret;
	struct name_message msg;

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Build operation header. */
	message_header_build(&msg.header, NAME_ALIVE);
	if ((ret = kernel_clock(&msg.op.heartbeat.timestamp)) < 0)
		return (ret);

	if ((ret = kmailbox_write(server, &msg, sizeof(struct name_message))) != sizeof(struct name_message))
		return (ret);

	return (0);
}

/*============================================================================*
 * nanvix_name_address_lookup()                                               *
 *============================================================================*/

/**
 * @todo TODO: provide a long description for this function.
 */
PUBLIC int nanvix_name_address_lookup(const char *name, int *port)
{
	int ret; /* Function return and remote_node during routine execution. */
	int nodenum;
	int outbox;
	struct name_message msg;

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_name_is_valid(name)) < 0)
		return (ret);

	/* Invalid port holder. */
	if (port == NULL)
		return (-EINVAL);

	/* @p name is not associated with any cluster. */
	if ((ret = nanvix_name_lookup(name)) < 0)
		return (ret);

	nodenum = ret;

	/* @p name is a local process. */
	if (nodenum == knode_get_num())
	{
		*port = _local_address_lookup(name);

		if ((*port) < 0)
			ret = (-ENOENT);

		return (ret);
	}

	/**
	 * Resolves @p name address consulting the remote cluster associated.
	 *
	 * @note Since IOClusters does not support a local name daemon to
	 * attend name resolution requests, this call may result in starvation for the
	 * current thread. Make sure about the cluster associated with @p name is
	 * a valid Compute Cluster.
	 *
	 * @todo Insert a security check to remove from the user the necessity of
	 * ensure the remote clusternum.
	 */

	/* Locks the port placeholder during an address lookup inquirement. */
	nanvix_mutex_lock(&_local_lock);

		/* Opens an outbox to the local name client. */
		if ((outbox = kmailbox_open(nodenum, NANVIX_NAME_SNOOPER_PORT_NUM)) < 0)
		{
			ret = outbox;
			goto end;
		}

		/* Build operation header. */
		message_header_build(&msg.header, NAME_ADDR);
		ustrcpy(msg.op.lookup.name, name);

		/* Notifies the local name daemon about the address_lookup to be done. */
		if ((ret = kmailbox_write(outbox, &msg, sizeof(struct name_message))) != sizeof(struct name_message))
			goto end;

again:
		/* Waits for the name snooper to sinalize that the remote lookup is done. */
		uassert(nanvix_cond_wait(&_local_condvar, &_local_lock) == 0);

		/* Not the expected process that was signaled? */
		if (ustrcmp(_addr_lookup_result.name, name) != 0)
		{
			if (ustrcmp(_addr_lookup_result.name, "") == 0)
			{
				ret = (-EAGAIN);
				goto end;
			}

			/* Wakes the next thread. */
			nanvix_cond_signal(&_local_condvar);
			goto again;
		}

		/* Retrieves the port number returned by the remote client. */
		*port = _addr_lookup_result.port_nr;
		ret = nodenum;

		/* Remote lookup was successful? */
		if ((*port) < 0)
			ret = (-ENOENT);

		/* Resets the shared variable. */
		_addr_lookup_result.port_nr = -1;
		ustrcpy(_addr_lookup_result.name, "");
		_addr_lookup_result.read    = 1;

end:
	nanvix_mutex_unlock(&_local_lock);

	uassert(kmailbox_close(outbox) == 0);

	return (ret);
}

/*============================================================================*
 * nanvix_name_register()                                                     *
 *============================================================================*/

/**
 * @todo TODO: provide a long description for this function.
 */
PUBLIC int nanvix_name_register(const char *name, int port_nr)
{
	int ret; /* Function return. */

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_name_is_valid(name)) < 0)
		return (ret);

	/* Invalid port number. */
	if ((port_nr < 0) || (port_nr >= MAILBOX_PORT_NR))
		return (-EINVAL);

	/* Name already registered? */
	if (_local_address_lookup(name) != (-1))
		return (-EAGAIN);

	/* Tries a remote name link. */
	if ((ret = nanvix_name_link(knode_get_num(), name)) != 0)
		return (ret);

	/* Locks the runtime to avoid data overwriting. */
	uassert(nanvix_mutex_lock(&_local_lock) == 0);

		/**
		 * Registers @p name in the local translation table.
		 *
		 * @note If an error occurs, unlink @p name in the remote server.
		 */
		if ((ret = _local_name_register(port_nr, name)) != 0)
			uassert(nanvix_name_unlink(name) == 0);

	uassert(nanvix_mutex_unlock(&_local_lock) == 0);

	return (ret);
}

/*============================================================================*
 * nanvix_name_unregister()                                                   *
 *============================================================================*/

/**
 * @todo TODO: provide a long description for this function.
 */
PUBLIC int nanvix_name_unregister(const char *name)
{
	int ret; /* Function return. */

	ret = (-EAGAIN);

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_name_is_valid(name)) < 0)
		return (ret);

	/* @p name was not previously registered. */
	if (_local_address_lookup(name) < 0)
		return (-EAGAIN);

	/* Unlinks @p name in the remote server. */
	if ((ret = nanvix_name_unlink(name)) < 0)
		return (ret);

	uassert(nanvix_mutex_lock(&_local_lock) == 0);

	/* Removes @p name from the local resolution table. */
	_local_name_unregister(name);

	uassert(nanvix_mutex_unlock(&_local_lock) == 0);

	return (0);
}

/*============================================================================*
 * nanvix_name_shutdown()                                                     *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_name_shutdown(void)
{
	int ret;
	struct name_message msg;

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Build operation header. */
	message_header_build(&msg.header, NAME_EXIT);

	if ((ret = kmailbox_write(server, &msg, sizeof(struct name_message))) != sizeof(struct name_message))
		return (ret);

	return (0);
}

/*============================================================================*
 * nanvix_name_snooper()                                                      *
 *============================================================================*/

PRIVATE int _do_name_query(struct name_message *req)
{
	UNUSED(req);
	return (0);
}

PRIVATE int _do_attend_lookup(struct name_message *req)
{
	UNUSED(req);
	return (0);
}

/**
 * @brief Name resolution local snooper.
 *
 * @param args Arguments for the thread (unused).
 *
 * @returns Always return NULL.
 */
static void * nanvix_name_snooper(void *args)
{
	struct name_message request;
	struct name_message response;
	const char *name;
	int port_nr;
	int outbox;

	UNUSED(args);

	uassert(__stdsync_setup() == 0);
	uassert(__stdmailbox_setup() == 0);
	uassert(__stdportal_setup() == 0);

	uprintf("[nanvix][name] name snooper listening port %d",
		stdinbox_get_port()
	);

	while (1)
	{
		/* Gets the remote client requisition. */
		uassert(
			kmailbox_read(
				stdinbox_get(),
				&request,
				sizeof(struct name_message)
			) == sizeof(struct name_message)
		);

		/* Checks if requisition is a local query or a remote lookup. */
		switch (request.header.opcode)
		{
			case NAME_SUCCESS:
#if DEBUG
		uprintf("[nanvix][name] name resolution answer received");
#endif /* DEBUG */
		again1:
				uassert(nanvix_mutex_lock(&_local_lock) == 0);

					/* Ensure that the last lookup was already consumed. */
					if (!_addr_lookup_result.read)
					{
						nanvix_mutex_unlock(&_local_lock);
						goto again1;
					}

					_addr_lookup_result.port_nr = request.op.addr_ans.port_nr;
					ustrcpy(_addr_lookup_result.name, request.op.addr_ans.name);
					_addr_lookup_result.read = 0;

				uassert(nanvix_mutex_unlock(&_local_lock) == 0);

				/* Wakes the requester up. */
				uassert(nanvix_cond_signal(&_local_condvar) == 0);

				break;

			case NAME_FAIL:
#if DEBUG
		uprintf("[nanvix][name] name resolution answer failed");
#endif /* DEBUG */
				uassert(nanvix_mutex_lock(&_local_lock) == 0);

					_addr_lookup_result.port_nr = -1;
					ustrcpy(_addr_lookup_result.name, "");
					_addr_lookup_result.read = 0;

				uassert(nanvix_mutex_unlock(&_local_lock) == 0);

				/* Wakes the requester up. */
				uassert(nanvix_cond_signal(&_local_condvar) == 0);

				break;

			case NAME_ADDR:
#if DEBUG
		uprintf("[nanvix][name] resolution requisition received");
#endif /* DEBUG */

				/* Opens output mailbox to the communicant peer. */
				uassert((
					outbox = kmailbox_open(
						request.header.source,
						NANVIX_NAME_SNOOPER_PORT_NUM
					)) >= 0
				);

				name = request.op.lookup.name;

				/* Lookup to the local resolution table to find @p name address. */
				port_nr = _local_address_lookup(name);

				/* Builds the response header. */
				message_header_build(
					&response.header,
					(port_nr < 0) ? NAME_FAIL : NAME_SUCCESS
				);

				response.op.addr_ans.port_nr = port_nr;
				ustrcpy(response.op.addr_ans.name, name);

				/* Sends the response back to the requisitant client. */
				uassert(
					kmailbox_write(
						outbox,
						&response,
						sizeof(struct name_message)
					) == sizeof(struct name_message)
				);

				/* Closes the opened outbox. */
				uassert(kmailbox_close(outbox) == 0);

				break;

			default:
				UNREACHABLE();
		}

#if DEBUG
		uprintf("[nanvix][name] resolution requisition attended");
#endif
	}

	return (NULL);
}

/*============================================================================*
 * __nanvix_name_setup()                                                      *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_name_setup(void)
{
	/* Nothing to do. */
	if (initialized)
		return (0);

	/* Open connection with Name Server. */
	if ((server = kmailbox_open(NAME_SERVER_NODE, NAME_SERVER_PORT_NUM)) < 0)
		return (-1);

	uassert(nanvix_mutex_init(&_name_lock) == 0);

	initialized = true;

	return (0);
}

/*============================================================================*
 * __nanvix_name_daemon_init()                                                *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_name_daemon_init(void)
{
	/* Nothing to do. */
	if (nanvix_name_snooper_tid >= 0)
		return (0);

	/**
	 * @brief Initialize local locking structures.
	 *
	 * @note These structures are initialized here because only cclusters can
	 * emmit address_lookups and only in these cases these structures may become
	 * necessary.
	 */
	uassert(nanvix_mutex_init(&_local_lock) == 0);

	uassert(nanvix_cond_init(&_local_condvar) == 0);

	/* Creates the name snooper thread. */
	uassert(kthread_create(&nanvix_name_snooper_tid, &nanvix_name_snooper, NULL) == 0);

	return (0);
}

/*============================================================================*
 * nanvix_name_cleanup()                                                      *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_name_cleanup(void)
{
	/* Nothing to do. */
	if (!initialized)
		return (0);

	/* Close connection with Name Server. */
	if (kmailbox_close(server) < 0)
		return (-EAGAIN);

	initialized = false;

	return (0);
}
