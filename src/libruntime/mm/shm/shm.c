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
#define __NEED_RESOURCE
#define __NEED_MM_SHM_SERVER
#define __NEED_MM_RMEM_CACHE

#include <nanvix/runtime/pm.h>
#include <nanvix/runtime/mm.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/thread.h>
#include <nanvix/config.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/sys/stat.h>
#include <posix/errno.h>
#include <posix/fcntl.h>

/**
 * @brief ID of snooper thread.
 */
static kthread_t nanvix_shm_snooper_tid;

/**
 * @brief Connection to SHMem Server.
 */
static struct
{
	int initialized; /**< Is the connection initialized? */
	int outbox;      /**< Output mailbox for requests.   */
} server = {
	.initialized = 0,
	.outbox = -1
};

/**
 * Table of opened shared memory regions (cache).
 */
static struct oregion
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource;       /**< Generic Resource Information  */

	int shmid;                      /**< ID of Underling SHM Region    */
	int oflags;                     /**< Opening flags.                */
	mode_t mode;                    /**< Acess Permissions             */
	int refcount;                   /**< References Count              */
	char name[NANVIX_SHM_NAME_MAX]; /**< Name of underlying shm region */
	rpage_t page;                   /**< Underlying Page               */
} oregions[NANVIX_SHM_OPEN_MAX];

/**
 * @brief Pool of open shared memory regions.
 */
struct resource_pool pool = {
	oregions, NANVIX_SHM_OPEN_MAX, sizeof(struct oregion)
};

/*============================================================================*
 * __nanvix_shm_lookup_name()                                                 *
 *============================================================================*/

/**
 * @brief Searches for a locally opened shared memory region by name.
 *
 * @param name Name of the target shared memory region.
 *
 * @returns If the requested shared memory region is found, its index in
 * the local table of opened shared memory regions. Otherwise, a
 * negative number is returned instead.
 */
static int shm_lookup_name(const char *name)
{
	for (int i = 0; i < NANVIX_SHM_OPEN_MAX; i++)
	{
		/* Skip invalid entries. */
		if (!resource_is_used(&oregions[i].resource))
			continue;

		/* Found. */
		if (!ustrcmp(oregions[i].name, name))
			return (i);
	}

	return (-1);
}

/*============================================================================*
 * __nanvix_shm_lookup_shmid()                                                 *
 *============================================================================*/

/**
 * @brief Searches for a locally opened shared memory region by ID.
 *
 * @param shmid Name of the target shared memory region.
 *
 * @returns If the requested shared memory region is found, its index in
 * the local table of opened shared memory regions. Otherwise, a
 * negative number is returned instead.
 */
static int shm_lookup_shmid(int shmid)
{
	for (int i = 0; i < NANVIX_SHM_OPEN_MAX; i++)
	{
		/* Skip invalid entries. */
		if (!resource_is_used(&oregions[i].resource))
			continue;

		/* Found. */
		if (oregions[i].shmid == shmid)
			return (i);
	}

	return (-1);
}

/*============================================================================*
 * OREGION_INITIALIZER()                                                      *
 *============================================================================*/

/**
 * @brief Initializes an opened shared memory region.
 *
 * @param shmid  ID of the underlying shared memory region.
 * @param oshmid ID of the target opened shared memory region.
 * @param name   Name of the target opened shared memory region
 * @param flags  Opening flags of the target shared memory region.
 * @param mode   Access permissions on the target shared memory region.
 */
static void __nanvix_shm_initializer(
	int oshmid,
	int shmid,
	const char *name,
	int oflags,
	mode_t mode
)
{
	oregions[oshmid].shmid = shmid;
	oregions[oshmid].oflags = oflags;
	oregions[oshmid].mode = mode;
	oregions[oshmid].refcount = 1;
	ustrcpy(oregions[oshmid].name, name);
	switch (oflags  & O_ACCMODE)
	{
		case O_RDWR:
			resource_set_rdwr(&oregions[oshmid].resource);
			break;

		case O_WRONLY:
			resource_set_wronly(&oregions[oshmid].resource);
			break;

		case O_RDONLY:
		default:
			resource_set_rdonly(&oregions[oshmid].resource);
			break;
	}
}

/*============================================================================*
 * nanvix_shm_truncate()                                                      *
 *============================================================================*/

/**
 * @brief Truncates a shared memory region to a size.
 *
 * @param shmid ID of the target shared memory region.
 * @param size  New size for the target shared memory region.
 *
 * @returns Upon sucessful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int __do_nanvix_shm_ftruncate(int shmid, off_t size)
{
	int oshmid;             /* ID of Opened Shared Memory Region */
	struct shm_message msg; /* Message to SHM Server             */

	/* Shared memory region is not opened. */
	if ((oshmid = shm_lookup_shmid(shmid)) < 0)
		return (-ENOENT);

	/* Shared memory region shall be removed soon. */
	if (oregions[oshmid].refcount == 0)
		return (-ENOENT);

	/* Build message.*/
	message_header_build(&msg.header, SHM_FTRUNCATE);
	msg.op.ftruncate.shmid = shmid;
	msg.op.ftruncate.size = size;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to truncate shared memory region. */
	if (msg.header.opcode == SHM_FAIL)
	{
		resource_free(&pool, oshmid);
		return (msg.op.ret.status);
	}

	oregions[oshmid].page = msg.op.ret.page;

	return (0);
}

/**
 * @see _do_nanvix_shm_ftruncate().
 */
int __nanvix_shm_ftruncate(int shmid, off_t size)
{
	/* Uninitialized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid ID. */
	if (!WITHIN(shmid, 0, NANVIX_SHM_MAX))
		return (-EINVAL);

	/* Invalid size. */
	if (size < 0)
		return (-EINVAL);

	/* Size too big. */
	if (size > NANVIX_SHM_SIZE_MAX)
		return (-EFBIG);

	return (__do_nanvix_shm_ftruncate(shmid, size));
}

/*============================================================================*
 * __do_nanvix_shm_create()                                                   *
 *============================================================================*/

/**
 * @brief Creates a shared memory region.
 *
 * @param name     Name of the target shared memory region.
 * @param oflags Opening flags for the target shared memory region.
 * @param mode     Access mode for the target shared memory region.
 *
 * @returns Upon successful completion the ID of the newly opened shared
 * memory region is returned. Upon failure, a negative error code is
 * returned instead.
 */
static int __do_nanvix_shm_create(const char *name, int oflags, mode_t mode)
{
	int shmid;              /* ID of the Underlying Shared Memory Region */
	int oshmid;             /* ID of Opened Shared Memory Region         */
	struct shm_message msg; /* Message to SHM Server                     */

	/*
	 * Allocate an entry in the local
	 * table of opened shared memory regions.
	 */
	if ((oshmid = resource_alloc(&pool)) < 0)
		return (-ENFILE);

	if (!(oflags & (O_RDWR | O_WRONLY)))
	{
		resource_free(&pool, oshmid);
		return (-EACCES);
	}

	/* Build message.*/
	message_header_build(&msg.header, SHM_CREATE);
	ustrcpy(msg.op.create.name, name);
	msg.op.create.oflags = oflags;
	msg.op.create.mode = mode;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to open shared memory region. */
	if (msg.header.opcode == SHM_FAIL)
	{
		resource_free(&pool, oshmid);
		return (msg.op.ret.status);
	}

	shmid = msg.op.ret.shmid;

	/*
	 * Initialize entry in local table of
	 * opened shared memory regions.
	 */
	oregions[oshmid].page = msg.op.ret.page;
	__nanvix_shm_initializer(oshmid, shmid, name, oflags, mode);

	return (shmid);
}

/*============================================================================*
 * __nanvix_shm_open()                                                        *
 *============================================================================*/

/**
 * @brief Opens a shared memory region.
 *
 * @param name   Name of the target shared memory region.
 * @param oflags Opening flags for the target shared memory region.
 * @param mode   Access mode for the target shared memory region.
 *
 * @returns Upon successful completion the ID of the opened shared
 * memory region is returned. Upon failure, a negative error code is
 * returned instead.
 */
static int __do_nanvix_shm_open(const char *name, int oflags, mode_t mode)
{
	int shmid;              /* ID of the Underlying Shared Memory Region */
	int oshmid;             /* ID of Opened Shared Memory Region         */
	struct shm_message msg; /* Message to SHM Server                     */

	/* Already opened. */
	if ((oshmid = shm_lookup_name(name)) >=  0)
	{
		int ret;

		/* Exclusive create. */
		if (oflags & O_EXCL)
			return (-EEXIST);

		/* Truncate? */
		if (oflags & O_TRUNC)
		{
			/* Cannot truncate. */
			if (!(oflags & (O_WRONLY | O_RDWR)))
				return (-EACCES);

			/* Truncate. */
			if ((ret = __do_nanvix_shm_ftruncate(oregions[oshmid].shmid, 0)) < 0)
				return (ret);
		}

		oregions[oshmid].refcount++;

		return (oshmid);
	}

	/* Create shared memory region. */
	if (oflags & O_CREAT)
		return (__do_nanvix_shm_create(name, oflags, mode));

	/* Truncate? */
	if (oflags & O_TRUNC)
	{
		/* Cannot write. */
		if (!(oflags & (O_RDWR | O_WRONLY)))
			return (-EACCES);
	}

	/*
	 * Allocate an entry in the local
	 * table of opened shared memory regions.
	 */
	if ((oshmid = resource_alloc(&pool)) < 0)
		return (-ENFILE);

	/* Build message.*/
	message_header_build(&msg.header, SHM_OPEN);
	ustrcpy(msg.op.open.name, name);
	msg.op.open.oflags = oflags;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to open shared memory region. */
	if (msg.header.opcode == SHM_FAIL)
	{
		resource_free(&pool, oshmid);
		return (msg.op.ret.status);
	}

	shmid = msg.op.ret.shmid;

	/*
	 * Initialize entry in local table of
	 * opened shared memory regions.
	 */
	oregions[oshmid].page = msg.op.ret.page;
	__nanvix_shm_initializer(oshmid, shmid, name, oflags, mode);

	return (shmid);
}

/**
 * @see __do_nanvix_shm_close()
 */
int __nanvix_shm_open(const char *name, int oflags, mode_t mode)
{
	int ret;

	/* Uninitialized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_shm_name_is_invalid(name)))
		return (ret);

	/* Invalid opening flags. */
	if (oflags & ~(O_CREAT | O_EXCL | O_TRUNC | O_ACCMODE))
		return (-ENOTSUP);

	/* Invalid access permission flags. */
	if (mode & ~(S_IRUSR | S_IWUSR))
		return (-ENOTSUP);

	return (__do_nanvix_shm_open(name, oflags, mode));
}

/*============================================================================*
 * __nanvix_shm_unlink()                                                      *
 *============================================================================*/

/**
 * @brief Removes a shared memory region.
 *
 * @param name Name of the target existing shared memory region.
 *
 * @returns Upon successful completion, a descriptor for the target
 * shared memory region is returned. Upon failure, a negative error
 * code is returned instead.
 */
static int __do_nanvix_shm_unlink(const char *name)
{
	int oshmid;             /* ID of Opened Shared Memory Region */
	struct shm_message msg; /* Message to SHM Server             */

	/* Invalid shared memory region. */
	if ((oshmid = shm_lookup_name(name)) < 0)
		return (-ENOENT);

	/* Shared memory region is busy. */
	if (oregions[oshmid].refcount >= 1)
		return (-EBUSY);

	/* Build message. */
	message_header_build(&msg.header, SHM_UNLINK);
	ustrcpy(msg.op.unlink.name, name);

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to unlink shared memory region. */
	if (msg.header.opcode == SHM_FAIL)
		return (msg.op.ret.status);

	/* Release region. */
	resource_free(&pool, oshmid);

	return (0);
}

/**
 * @see __do_nanvix_shm_unlink()
 */
int __nanvix_shm_unlink(const char *name)
{
	int ret;

	/* Uninitialized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid name. */
	if ((ret = nanvix_shm_name_is_invalid(name)))
		return (ret);


	return (__do_nanvix_shm_unlink(name));
}

/*============================================================================*
 * __nanvix_shm_close()                                                       *
 *============================================================================*/

/**
 * @brief Closes a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 *
 * @returns Upon sucessful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int __do_nanvix_shm_close(int shmid)
{
	int oshmid; /* ID of Opened Shared Memory Region */

	/* Invalid shared memory region. */
	if ((oshmid = shm_lookup_shmid(shmid)) < 0)
		return (-ENOENT);

	/* Bad memory region. */
	if (oregions[oshmid].refcount == 0)
		return (-ENOENT);

	/* Release remote shared memory region. */
	if (oregions[oshmid].refcount == 1)
	{
		struct shm_message msg;

		/* Build message. */
		message_header_build(&msg.header, SHM_CLOSE);
		msg.op.close.shmid = shmid;

		/* Send operation. */
		uassert(
			nanvix_mailbox_write(
				server.outbox,
				&msg,
				sizeof(struct shm_message)
			) == 0
		);

		/* Receive reply. */
		uassert(
			kmailbox_read(
				stdinbox_get(),
				&msg,
				sizeof(struct shm_message)
			) == sizeof(struct shm_message)
		);

		/* Failed to close shared memory region. */
		if (msg.header.opcode == SHM_FAIL)
			return (msg.op.ret.status);
	}

	/* Release region. */
	oregions[oshmid].refcount--;
	if ((oregions[oshmid].refcount == 0) && !(oregions[oshmid].oflags & O_CREAT))
		resource_free(&pool, oshmid);

	return (0);
}

/**
 * @see __do_nanvix_shm_close()
 */
int __nanvix_shm_close(int shmid)
{
	/* Uninitialized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid ID. */
	if (!WITHIN(shmid, 0, NANVIX_SHM_MAX))
		return (-EINVAL);

	return (__do_nanvix_shm_close(shmid));
}

/*============================================================================*
 * __nanvix_shm_read()                                                        *
 *============================================================================*/

/**
 * @brief Reads data from a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 * @param buf   Location where the data should be written to.
 * @param n     Number of bytes to read.
 * @param off   Read offset within the shared memory region.
 *
 * @returns Upon successful completion, the number of bytes actualy
 * read is returned. Upon failure, a negative error code is returned
 * instead.
 */
static ssize_t __do_nanvix_shm_read(int shmid, void *buf, size_t n, off_t off)
{
	char *ptr;  /* Local page.                       */
	int oshmid; /* ID of Opened Shared Memory Region */

	/* Invalid shared memory region. */
	if ((oshmid = shm_lookup_shmid(shmid)) < 0)
		return (-ENOENT);

	/* Bad memory region. */
	if (oregions[oshmid].refcount == 0)
		return (-ENOENT);

	/* Bad memory region. */
	if (oregions[oshmid].page == RMEM_NULL)
		return (-ENOMEM);

	((void) off);

	uassert((ptr = nanvix_rcache_get(oregions[oshmid].page)) != NULL);
	umemcpy(buf, ptr + off, n);
	uassert(nanvix_rcache_put(oregions[oshmid].page, 1) == 0);

	return (n);
}

/**
 * @see __do_nanvix_shm_read()
 */
ssize_t __nanvix_shm_read(int shmid, void *buf, size_t n, off_t off)
{
	/* Uninitialized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid ID. */
	if (!WITHIN(shmid, 0, NANVIX_SHM_MAX))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid read size. */
	if (n != NANVIX_SHM_SIZE_MAX)
		return (-EINVAL);

	/* Invalid offset. */
	if ((off < 0) || ((n + off) > NANVIX_SHM_SIZE_MAX))
		return (-EINVAL);

	return (__do_nanvix_shm_read(shmid, buf, n, off));
}

/*============================================================================*
 * __nanvix_shm_write()                                                       *
 *============================================================================*/

/**
 * @brief Writes data to a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 * @param buf   Location where the data should be read from.
 * @param n     Number of bytes to write.
 * @param off   Read offset within the shared memory region.
 *
 * @returns Upon successful completion, the number of bytes actualy
 * written is returned. Upon failure, a negative error code is
 * returned instead.
 */
static ssize_t __do_nanvix_shm_write(int shmid, const void *buf, size_t n, off_t off)
{
	char *ptr;  /* Local page.                       */
	int oshmid; /* ID of Opened Shared Memory Region */

	/* Invalid shared memory region. */
	if ((oshmid = shm_lookup_shmid(shmid)) < 0)
		return (-ENOENT);

	/* Bad memory region. */
	if (oregions[oshmid].refcount == 0)
		return (-ENOENT);

	/* Bad memory region. */
	if (oregions[oshmid].page == RMEM_NULL)
		return (-ENOMEM);

	((void) off);

	uassert((ptr = nanvix_rcache_get(oregions[oshmid].page)) != NULL);
	umemcpy(ptr + off, buf, n);
	uassert(nanvix_rcache_put(oregions[oshmid].page, 1) == 0);

	return (n);
}

/**
 * @see __do_nanvix_shm_write()
 */
ssize_t __nanvix_shm_write(int shmid, const void *buf, size_t n, off_t off)
{
	/* Invalid ID. */
	if (!WITHIN(shmid, 0, NANVIX_SHM_MAX))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid read size. */
	if (n != NANVIX_SHM_SIZE_MAX)
		return (-EINVAL);

	/* Invalid offset. */
	if ((off < 0) || ((n + off) > NANVIX_SHM_SIZE_MAX))
		return (-EINVAL);

	return (__do_nanvix_shm_write(shmid, buf, n, off));
}

/*============================================================================*
 * __nanvix_shm_inval()                                                       *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
static int __do_nanvix_shm_inval(int shmid)
{
	int oshmid;
	struct shm_message msg;

	/* Invalid shared memory region. */
	if ((oshmid = shm_lookup_shmid(shmid)) < 0)
		return (-ENOENT);

	/* Bad memory region. */
	if (oregions[oshmid].refcount == 0)
		return (-ENOENT);

	/* Build message. */
	message_header_build(&msg.header, SHM_INVAL);
	msg.op.inval.page = oregions[oshmid].page;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct shm_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct shm_message)
		) == sizeof(struct shm_message)
	);

	/* Failed to close shared memory region. */
	if (msg.header.opcode == SHM_FAIL)
		return (msg.op.ret.status);

	return (0);
}

/**
 * @see __do_nanvix_shm_inval()
 */
int __nanvix_shm_inval(int shmid)
{
	/* Uninitialized server. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid ID. */
	if (!WITHIN(shmid, 0, NANVIX_SHM_MAX))
		return (-EINVAL);

	return (__do_nanvix_shm_inval(shmid));
}

/*============================================================================*
 * nanvix_shm_snooper()                                                       *
 *============================================================================*/

/**
 * @brief Shared memory region snooper.
 *
 * @param args Arguments for the thread (unused).
 *
 * @returns Always return NULL.
 */
static void *nanvix_shm_snooper(void *args)
{
	struct shm_message msg;

	UNUSED(args);

	uassert(__stdsync_setup() == 0);
	uassert(__stdmailbox_setup() == 0);
	uassert(__stdportal_setup() == 0);

	uprintf("[nanvix][shm] snooper listening port %d",
		stdinbox_get_port()
	);

	while (1)
	{
		uassert(
			kmailbox_read(
				stdinbox_get(),
				&msg,
				sizeof(struct shm_message)
			) == sizeof(struct shm_message)
		);

		uprintf("[nanvix][shm] invalidation signal received");
	}

	return (NULL);
}

/*============================================================================*
 * nanvix_shm_shutdown()                                                      *
 *============================================================================*/

/**
 * The nanvix_shm_shutdown() function issues the shutdown signal to the
 * SHM server, thus causing the server to terminate.
 */
int nanvix_shm_shutdown(void)
{
	struct shm_message msg;

	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Build operation header. */
	message_header_build(&msg.header, SHM_EXIT);

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct shm_message)
		) == 0
	);

	return (0);
}

/*============================================================================*
 * __nanvix_shm_setup()                                                       *
 *============================================================================*/

/**
 * The __nanvhx_shm_setup() function initializes the local daemon for
 * the SHM service.
 */
int __nanvix_shm_setup(void)
{
	/* Nothing to do.  */
	if (server.initialized)
		return (0);

	/* Initialize table of shared memory regions. */
	for (int i = 0; i < NANVIX_SHM_OPEN_MAX; i++)
	{
		oregions[i].shmid = -1;
		oregions[i].resource = RESOURCE_INITIALIZER;
		ustrcpy(oregions[i].name, "");
	}

	/* Open output mailbox */
	if ((server.outbox = nanvix_mailbox_open(SHM_SERVER_NAME, SHM_SERVER_PORT_NUM)) < 0)
	{
		uprintf("[nanvix][shm] cannot open outbox to server");
		return (server.outbox);
	}

	server.initialized = true;
	uprintf("[nanvix][shm] connection with server established");

	uassert(kthread_create(&nanvix_shm_snooper_tid, &nanvix_shm_snooper, NULL) == 0);

	return (0);
}

/*============================================================================*
 * __nanvix_shm_cleanup()                                                     *
 *============================================================================*/

/**
 * The __nanvix_shm_cleanup() function shutdowns the local daemon for
 * the SHM service.
 */
int __nanvix_shm_cleanup(void)
{
	int ret;

	/* Nothing to do. */
	if (!server.initialized)
		return (0);

	/* Close output mailbox. */
	if ((ret = nanvix_mailbox_close(server.outbox)) < 0)
	{
		uprintf("[nanvix][shm] cannot close outbox to server");
		return (ret);
	}

	server.initialized = 0;

	return (0);
}
