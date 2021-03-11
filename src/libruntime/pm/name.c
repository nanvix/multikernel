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
#include <nanvix/sys/semaphore.h>
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

//PRIVATE struct nanvix_cond_var _local_condvar;

/**
 * @brief Number of address cache lines.
 */
#define ADDR_CACHE_ENTRIES 4

struct addr_cache_value
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource;  /**< Generic resource information. */

	int remote;
	int port_nr;
} _values[ADDR_CACHE_ENTRIES];

/**
 * @brief Pool of request nodes.
 */
PRIVATE const struct resource_pool pool_values = {
	_values, ADDR_CACHE_ENTRIES, sizeof(struct addr_cache_value)
};

/**
 * @brief Address cache structure.
 */
PRIVATE struct
{
	struct
	{
		struct
		{
			char name[NANVIX_PROC_NAME_MAX]; /**< Entry key.                     */
			int value_id;                    /**< Entry value id in values pool. */
		} entry;

		int refcount;                        /**< Line refcounter. */
		struct nanvix_mutex lock;            /**< Line lock.       */
		struct nanvix_cond_var condvar;      /**< Line condvar.    */
	} table[ADDR_CACHE_ENTRIES];

	struct nanvix_mutex global_lock;
	struct nanvix_semaphore counter;
} _addr_cache;

/*============================================================================*
 * LOCAL CACHE OPERATIONS                                                     *
 *============================================================================*/

/**
 * @brief Initializes the addr cache structure.
 *
 * @returns Upon successful completion, zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
PRIVATE int _addr_cache_init(void)
{
	/* Initializes the address cache entries. */
	for (int i = 0; i < ADDR_CACHE_ENTRIES; ++i)
	{
		/* Initializes individual lock of the cache entry. */
		uassert(nanvix_mutex_init(&_addr_cache.table[i].lock, NULL) == 0);

		/* Initializes the inddividual condvar of each line. */
		uassert(nanvix_cond_init(&_addr_cache.table[i].condvar) == 0);

		/* Initializes the remaining entry attributes. */
		ustrcpy(_addr_cache.table[i].entry.name, "");
		_addr_cache.table[i].entry.value_id = -1;
		_addr_cache.table[i].refcount       = -1;
	}

	/* Initializes the address cache global lock. */
	uassert(nanvix_mutex_init(&_addr_cache.global_lock, NULL) == 0);

	/* Initializes the structure semaphore used as counter. */
	uassert(nanvix_semaphore_init(&_addr_cache.counter, 0) == 0);

	return (0);
}

/**
 * @brief Gets the cache entry value associated with a given name.
 *
 * @param name Process name to be used as key.
 *
 * @returns Upon successful completion, a struct containing the remote node
 * number and the remote port associated with @p name is returned. Upon failure,
 * the same struct is returned, but with a negative number in its internal
 * attributes is returned instead.
 *
 * @note On a hit, this routine also updates the refcount of the found index.
 */
PRIVATE struct addr_cache_value _addr_cache_get(const char *name)
{
	struct addr_cache_value ret;

	ret.remote  = -1;
	ret.port_nr = -1;

	//uprintf("Before global lock");

	/* Acquire the cache global lock. */
	uassert(nanvix_mutex_lock(&_addr_cache.global_lock) == 0);

		//uprintf("Got the lock");

		/* Sinalize that one more thread is searching for cache entries. */
		uassert(nanvix_semaphore_up(&_addr_cache.counter) == 0);

		//uprintf("Signalized an up in the semaphore");

	uassert(nanvix_mutex_unlock(&_addr_cache.global_lock) == 0);

	//uprintf("After global lock");

	/* Traverses the addr_cache structure. */
	for (int i = 0; i < ADDR_CACHE_ENTRIES; ++i)
	{
		/* Acquire the specific entry lock to consult its value. */
		uassert(nanvix_mutex_lock(&_addr_cache.table[i].lock) == 0);

			/* Valid entry? */
			if (ustrcmp(_addr_cache.table[i].entry.name, "") == 0)
				goto release;

			/* Found? */
			if (ustrcmp(_addr_cache.table[i].entry.name, name) != 0)
			{
				uassert(nanvix_mutex_unlock(&_addr_cache.table[i].lock) == 0);
				continue;
			}

			/* Value being retrieved? */
			if (_addr_cache.table[i].entry.value_id == -1)
				uassert(nanvix_cond_wait(&_addr_cache.table[i].condvar, &_addr_cache.table[i].lock) == 0);

			ret = _values[_addr_cache.table[i].entry.value_id];
			_addr_cache.table[i].refcount++;

	release:
		uassert(nanvix_mutex_unlock(&_addr_cache.table[i].lock) == 0);

		break;
	}

	//uprintf("Did not found an entry");

	/* Sinalize that the current thread is no longer consulting the cache. */
	uassert(nanvix_semaphore_down(&_addr_cache.counter) == 0);

	//uprintf("Returning from cache_get");

	return (ret);
}

/**
 * @brief Inserts a new key in the address cache.
 *
 * @param name  Process name to be used as key.
 * @param value Value to be associated with @p name.
 *
 * @returns Upon successful completion, the index where the given key was
 * inserted is returned. Upon failure, a negative error code is returned
 * instead.
 */
PRIVATE int _addr_cache_put(const char *name)
{
	int index;        /**< Index where the new entry will be stored. */
	int min_refcount;

	/* Acquire the cache global lock. */
	uassert(nanvix_mutex_lock(&_addr_cache.global_lock) == 0);

		/* Do some busy waiting here. */
		while (nanvix_semaphore_trywait(&_addr_cache.counter) == 0)
			uassert(nanvix_semaphore_up(&_addr_cache.counter) == 0);

		/**
		 * The busy waiting done in the last step guarantees that the current
		 * thread is the only one manipulating the cache table. From now on,
		 * we can manipulate it without worrying about concurrence.
		 */

		/* Traverses the cache table looking for a free or the least frequent entry. */
		for (int i = 0; i < ADDR_CACHE_ENTRIES; ++i)
		{
			/* Free entry? */
			if (ustrcmp(_addr_cache.table[i].entry.name, "") == 0)
			{
				index = i;
				break;
			}

			/* Smallest refcount until now? */
			if ((i == 0) || (_addr_cache.table[i].refcount < min_refcount))
			{
				index = i;
				min_refcount = _addr_cache.table[i].refcount;
			}

			/* Resets the entry refcount. */
			_addr_cache.table[i].refcount = 0;
		}

		/* Frees the resource allocated to the given index. */
		resource_free(&pool_values, _addr_cache.table[index].entry.value_id);
		_addr_cache.table[index].entry.value_id = -1;

		/* Inserts the new key in the index found in the last step. */
		ustrcpy(_addr_cache.table[index].entry.name, name);
		//_addr_cache.table[index].refcount++;

	/* Releases the global lock. */
	uassert(nanvix_mutex_unlock(&_addr_cache.global_lock) == 0);

	return (index);
}

/**
 * @brief Updates an existing entry value in the address cache.
 *
 * @param name  Process name to be used as key.
 * @param value Value to be associated with @p name.
 *
 * @returns Upón successful completion, the index that corresponds to the updated
 * key is returned. Upon failure, a negative error code is returned instead.
 *
 * @note A failure case is when the system try to update an unexisting key value.
 * Additionally, this routine can only be used to set the reference pointer
 * of an entry pointing to a NULL value reference.
 */
PRIVATE int _addr_cache_update(const char *name, struct addr_cache_value value)
{
	int value_id; /**< Index where the new entry will be stored. */

	/* Traverses the address cache looking for the given key. */
	for (int i = 0; i < ADDR_CACHE_ENTRIES; ++i)
	{
		/* Found? */
		if (ustrcmp(_addr_cache.table[i].entry.name, name) != 0)
			continue;

		/* This key already have a value associated? */
		if (_addr_cache.table[i].entry.value_id != -1)
			return (-EINVAL);

		/* Allocates a free value holder. */
		if ((value_id = resource_alloc(&pool_values)) < 0)
			return (-EAGAIN);

		_values[value_id] = value;
		_addr_cache.table[i].entry.value_id = value_id;

		return (i);
	}

	/* The given key does not exist in the addr_cache. */
	return (-EINVAL);
}

/*============================================================================*
 * LOCAL NAME OPERATIONS                                                      *
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
	int ret;
	int index;
	int value_id;
	int nodenum;
	int outbox;
	int may_be_local;
	struct name_message msg;
	struct addr_cache_value cache_value;

	may_be_local = 1;

	/* Initilize name client. */
	if (!initialized)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_name_is_valid(name)) < 0)
		return (ret);

	/* Invalid port holder. */
	if (port == NULL)
		return (-EINVAL);

again:
	/* Checks local address cache first. */
	//uprintf("Searching for address in cache.");

	cache_value = _addr_cache_get(name);

	/* Valid reference was found? */
	if ((ret = cache_value.remote) > 0)
	{
		*port = cache_value.port_nr;
		return (cache_value.remote);
	}

	//uprintf("No valid reference found");

	/* Already testes if it is a local node? */
	if (may_be_local)
	{
		/* @p name is not associated with any cluster. */
		if ((nodenum = nanvix_name_lookup(name)) < 0)
			return (nodenum);

		/* @p name is a local process. */
		if (nodenum == knode_get_num())
		{
			*port = _local_address_lookup(name);
			ret   = nodenum;

			if ((*port) < 0)
				ret = (-ENOENT);

			return (ret);
		}

		/* Sinalizes the thread to not inquire about locality anymore. */
		may_be_local = 0;
	}

	//uprintf("Not a local process");

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

	/* Is there another thread already realizing an address requisition? */
	if (nanvix_mutex_trylock(&_local_lock) != 0)
	{
		goto again;
	}

		/* From now on we have the local lock to send an address requisition. */

		/* Puts the new key in the addr_cache. */
		uassert((index = _addr_cache_put(name)) >= 0);

		/* Opens an outbox to the local name client. */
		if ((outbox = kmailbox_open(nodenum, NANVIX_NAME_SNOOPER_PORT_NUM)) < 0)
		{
			ret = outbox;
			goto end;
		}

		/* Build operation header. */
		message_header_build(&msg.header, NAME_ADDR);
		ustrcpy(msg.op.lookup.name, name);

		/**
		 * Acquire the specific entry lock to be released in cond_wait.
		 *
		 * @note This lock need to be before the mailbox_write to avoid the
		 * address_lookup result to arrive before the current thread registers
		 * itself to wait in the specific condition variable.
		 */
		uassert(nanvix_mutex_lock(&_addr_cache.table[index].lock) == 0);

			/* Notifies the local name daemon about the address_lookup to be done. */
			if ((ret = kmailbox_write(outbox, &msg, sizeof(struct name_message))) != sizeof(struct name_message))
				goto unlock_entry;

			/* Waits for the name snooper to sinalize that the remote lookup is done. */
			uassert(nanvix_cond_wait(&_addr_cache.table[index].condvar, &_addr_cache.table[index].lock) == 0);

			/* Not the expected process that was signaled? */
			// if (ustrcmp(_addr_lookup_result.name, name) != 0)
			// {
			// 	if (ustrcmp(_addr_lookup_result.name, "") == 0)
			// 	{
			// 		ret = (-EAGAIN);
			// 		goto end;
			// 	}

			// 	/* Wakes the next thread. */
			// 	nanvix_cond_signal(&_local_condvar);
			// 	goto again;
			// }

			/* Security check. */
			uassert(ustrcmp(_addr_cache.table[index].entry.name, name) == 0);
			value_id = _addr_cache.table[index].entry.value_id;

			/* Retrieves the port number returned by the remote client. */
			*port = _values[value_id].port_nr;
			ret   = _values[value_id].remote;

			/* Remote lookup was successful? */
			if ((*port) < 0)
				ret = (-ENOENT);

			/* Resets the shared variable. */
			// _addr_lookup_result.port_nr = -1;
			// ustrcpy(_addr_lookup_result.name, "");
			// _addr_lookup_result.read    = 1;

unlock_entry:
		uassert(nanvix_mutex_unlock(&_addr_cache.table[index].lock) == 0);

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
	struct addr_cache_value value;
	int index;
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
				//uassert(nanvix_mutex_lock(&_local_lock) == 0);

				value.remote  = request.header.source;
				value.port_nr = request.op.addr_ans.port_nr;

				/* Updates the key with its new valuein the addres cache. */
				uassert((index = _addr_cache_update(request.op.addr_ans.name, value)) >= 0);

				// ustrcpy(_addr_lookup_result.name, );
				// _addr_lookup_result.read = 0;

				/* Wakes all threads waiting for this result. */
				uassert(nanvix_cond_broadcast(&_addr_cache.table[index].condvar) == 0);

				//uassert(nanvix_mutex_unlock(&_local_lock) == 0);

				break;

			case NAME_FAIL:
#if DEBUG
		uprintf("[nanvix][name] name resolution answer failed");
#endif /* DEBUG */
				value.remote  = -1;
				value.port_nr = -1;

				/* Updates the key with its new valuein the addres cache. */
				uassert((index = _addr_cache_update(request.op.addr_ans.name, value)) >= 0);

				/* Wakes all threads waiting for this result. */
				uassert(nanvix_cond_broadcast(&_addr_cache.table[index].condvar) == 0);

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

	uassert(nanvix_mutex_init(&_name_lock, NULL) == 0);

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
	uassert(nanvix_mutex_init(&_local_lock, NULL) == 0);

	/* Initializes the address cache structure. */
	uassert(_addr_cache_init() == 0);

	//uassert(nanvix_cond_init(&_local_condvar) == 0);

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
