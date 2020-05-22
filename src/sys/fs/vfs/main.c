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
#define __VFS_SERVER

#include <nanvix/servers/connection.h>
#include <nanvix/servers/vfs.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/sys/semaphore.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/noc.h>
#include <nanvix/config.h>
#include <nanvix/dev.h>
#include <nanvix/ulib.h>

/* Import definitions. */
extern void vfs_test(void);

/**
 * @brief VFS Server information.
 */
static struct
{
	int nodenum;  /**< Node number.  */
	int inbox;    /**< Input mailbox */
	int inportal; /**< Input Portal  */
	const char *name;
} server = {
	-1, -1, -1, VFS_SERVER_NAME
};

/*============================================================================*
 * vfs_loop()                                                                 *
 *============================================================================*/

/**
 * @brief Handles file system region requests.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_vfs_loop(void)
{
	int shutdown = 0;

	while (!shutdown)
	{
		int outbox;
		int reply = 0;
		int ret = -ENOSYS;
		struct vfs_message request;
		struct vfs_message response;

		uassert(
			kmailbox_read(
				server.inbox,
				&request,
				sizeof(struct vfs_message)
			) == sizeof(struct vfs_message)
		);

		vfs_debug("vfs request source=%d port=%d opcode=%d",
			request.header.source,
			request.header.portal_port,
			request.header.opcode
		);

		/* TODO check for bad node number. */

		/* Handle request. */
		switch (request.header.opcode)
		{
			case VFS_EXIT:
				shutdown = 1;
				break;

			case VFS_CREAT:
				reply = 1;
				break;

			case VFS_OPEN:
				reply = 1;
				break;

			case VFS_UNLINK:
				reply = 1;
				break;

			case VFS_CLOSE:
				reply = 1;
				break;

			case VFS_LINK:
				reply = 1;
				break;

			case VFS_TRUNCATE:
				reply = 1;
				break;

			case VFS_STAT:
				reply = 1;
				break;

			case VFS_READ:
				reply = 1;
				break;

			case VFS_WRITE:
				reply = 1;
				break;

			case VFS_SEEK:
				reply = 1;
				break;

			default:
				break;
		}

		/* No reply? */
		if (!reply)
			continue;

		response.op.ret.status = ret;
		message_header_build(
			&response.header,
			(ret < 0) ? VFS_FAIL : VFS_SUCCESS
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
				sizeof(struct vfs_message
			)) == sizeof(struct vfs_message)
		);
		uassert(kmailbox_close(outbox) == 0);
	}

	vfs_test();

	return (0);
}

/*============================================================================*
 * do_vfs_startup()                                                           *
 *============================================================================*/

/**
 * @brief Initializes the file system server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_vfs_startup(struct nanvix_semaphore *lock)
{
	int ret;

	uprintf("[nanvix][vfs] booting up server");

	server.nodenum = knode_get_num();
	server.inbox = stdinbox_get();
	server.inportal = stdinportal_get();

	/* Link name. */
	if ((ret = name_link(server.nodenum, server.name)) < 0)
		return (ret);

	connections_setup();
	vfs_init();

	uprintf("[nanvix][vfs] minix file system created");

	uprintf("[nanvix][vfs] server alive");
	uprintf("[nanvix][vfs] attached to node %d", server.nodenum);
	uprintf("[nanvix][vfs] listening to mailbox %d", server.inbox);
	uprintf("[nanvix][vfs] listening to portal %d", server.inportal);

	nanvix_semaphore_up(lock);

	return (0);
}

/*============================================================================*
 * do_vfs_shutdown()                                                          *
 *============================================================================*/

/**
 * @brief Shutdowns the file system server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_vfs_shutdown(void)
{
	uprintf("[nanvix][vfs] shutting down server");

	return (0);
}

/*============================================================================*
 * do_vfs_server()                                                            *
 *============================================================================*/

/**
 * @brief Virtual Files System server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_vfs_server(struct nanvix_semaphore *lock)
{
	int ret;

	if ((ret = do_vfs_startup(lock)) < 0)
	{
		uprintf("[nanvix][vfs] failed to startup server!");
		goto error;
	}

	if ((ret = do_vfs_loop()) < 0)
	{
		uprintf("[nanvix][vfs] failed to launch server!");
		goto error;
	}

	if ((ret = do_vfs_shutdown()) < 0)
	{
		uprintf("[nanvix][vfs] failed to shutdown server!");
		goto error;
	}

	return (0);

error:
	return (ret);
}

/*============================================================================*
 * vfs_server()                                                               *
 *============================================================================*/

/**
 * @brief Handles file system requests.
 *
 * @returns Always returns zero.
 */
int vfs_server(struct nanvix_semaphore *lock)
{
	uassert(do_vfs_server(lock) == 0);

	return (0);
}
