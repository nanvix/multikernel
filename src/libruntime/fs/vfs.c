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
#define __NEED_FS_VFS_SERVER

#include <nanvix/runtime/runtime.h>
#include <nanvix/servers/vfs.h>
#include <nanvix/config.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/**
 * @brief Connection to VFSem Server.
 */
static struct
{
	int initialized; /**< Is the connection initialized? */
	int outbox;      /**< Output mailbox for requests.   */
} server = {
	.initialized = 0,
	.outbox = -1
};

/*============================================================================*
 * nanvix_vfs_shutdown()                                                      *
 *============================================================================*/

/**
 * The nanvix_vfs_shutdown() function issues the shutdown signal to the
 * VFS server, thus causing the server to terminate.
 */
int nanvix_vfs_shutdown(void)
{
	struct vfs_message msg;

	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Build operation header. */
	message_header_build(&msg.header, VFS_EXIT);

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct vfs_message)
		) == 0
	);

	return (0);
}

/*============================================================================*
 * __nanvix_vfs_setup()                                                       *
 *============================================================================*/

/**
 * The __nanvhx_vfs_setup() function initializes the local daemon for
 * the VFS service.
 */
int __nanvix_vfs_setup(void)
{
	/* Nothing to do.  */
	if (server.initialized)
		return (0);

	/* Open output mailbox */
	if ((server.outbox = nanvix_mailbox_open(VFS_SERVER_NAME, VFS_SERVER_PORT_NUM)) < 0)
	{
		uprintf("[nanvix][vfs] cannot open outbox to server");
		return (server.outbox);
	}

	server.initialized = true;
	uprintf("[nanvix][vfs] connection with server established");

	return (0);
}

/*============================================================================*
 * __nanvix_vfs_cleanup()                                                     *
 *============================================================================*/

/**
 * The __nanvix_vfs_cleanup() function shutdowns the local daemon for
 * the VFS service.
 */
int __nanvix_vfs_cleanup(void)
{
	int ret;

	/* Nothing to do. */
	if (!server.initialized)
		return (0);

	/* Close output mailbox. */
	if ((ret = nanvix_mailbox_close(server.outbox)) < 0)
	{
		uprintf("[nanvix][vfs] cannot close outbox to server");
		return (ret);
	}

	server.initialized = 0;

	return (0);
}
