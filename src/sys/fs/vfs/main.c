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
#include <nanvix/runtime/runtime.h>
#include <nanvix/sys/semaphore.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/noc.h>
#include <nanvix/config.h>
#include <nanvix/dev.h>
#include <nanvix/fs.h>
#include <nanvix/types.h>
#include <nanvix/ulib.h>
#include <posix/sys/stat.h>

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

/**
 * @brief Buffer for Read/Write Requests
 */
static char buffer[NANVIX_FS_BLOCK_SIZE];

/*============================================================================*
 * do_vfs_server_stat()                                                       *
 *============================================================================*/

/**
 * @brief Handles an stat request.
 *
 * @param request  Target request.
 * @param response Response.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @author Lucca Augusto
 */
static int do_vfs_server_stat(
	const struct vfs_message *request,
	struct vfs_message *response
)
{
	int ret;
	const int port = request->header.mailbox_port;
	const nanvix_pid_t pid = request->header.source;
	const int connection = connect(pid, port);

	/* XXX: forward parameter checking to lower level function. */

	ret = vfs_stat(
		connection,
		request->op.stat.filename,
		&response->op.stat.buf
	);
	/* Operation failed. */
	if (ret < 0)
	{
		disconnect(pid, port);
		return (ret);
	}

	response->op.ret.fd = ret;

	return (0);
}

/*============================================================================*
 * do_vfs_server_open()                                                       *
 *============================================================================*/

/**
 * @brief Handles an open request.
 *
 * @param request  Target request.
 * @param response Response.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @author Pedro Henrique Penna
 */
static int do_vfs_server_open(
	const struct vfs_message *request,
	struct vfs_message *response
)
{
	int ret;
	const int port = request->header.mailbox_port;
	const nanvix_pid_t pid = request->header.source;
	const int connection = connect(pid, port);

	/* XXX: forward parameter checking to lower level function. */

	ret = vfs_open(
		connection,
		request->op.open.filename,
		request->op.open.oflag,
		request->op.open.mode
	);

	/* Operation failed. */
	if (ret < 0)
	{
		disconnect(pid, port);
		return (ret);
	}

	response->op.ret.fd = ret;

	return (0);
}

/*============================================================================*
 * do_vfs_server_close()                                                      *
 *============================================================================*/

/**
 * @brief Handles a close request.
 *
 * @param request Target request.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @author Pedro Henrique Penna
 */
static int do_vfs_server_close(const struct vfs_message *request)
{
	int ret;
	const int port = request->header.mailbox_port;
	const nanvix_pid_t pid = request->header.source;
	const int connection = lookup(pid, port);

	/* XXX: forwarding parameter checking to lower level function. */

	ret = vfs_close(
		connection,
		request->op.close.fd
	);

	/* Operation failed. */
	if (ret < 0)
		return (ret);

	disconnect(pid, port);

	return (0);
}

/*============================================================================*
 * do_vfs_server_unlink()                                                     *
 *============================================================================*/

/**
 * @brief Handles a unlink request.
 *
 * @param request Target request.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @author Lucca Augusto
 */
static int do_vfs_server_unlink(const struct vfs_message *request)
{
	int ret;
	const int port = request->header.mailbox_port;
	const nanvix_pid_t pid = request->header.source;
	int aux_conn = lookup(pid, port);

	/* file not open, open it */
	if ( aux_conn <= 0) {
		aux_conn = connect(pid, port);
	}

	const int connection = aux_conn;

	/* XXX: forwarding parameter checking to lower level function. */

	ret = vfs_unlink(
		connection,
		request->op.unlink.filename
	);

	/* Operation failed. */
	if (ret < 0)
		return (ret);

	disconnect(pid, port);

	return (0);
}

/*============================================================================*
 * do_vfs_server_seek()                                                       *
 *============================================================================*/

/**
 * @brief Handles an seek request.
 *
 * @param request  Target request.
 * @param response Response.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @author Pedro Henrique Penna
 */
static int do_vfs_server_seek(
	const struct vfs_message *request,
	struct vfs_message *response
)
{
	off_t ret;
	const int port = request->header.mailbox_port;
	const nanvix_pid_t pid = request->header.source;
	const int connection = lookup(pid, port);

	/* XXX: forward parameter checking to lower level function. */

	ret = vfs_seek(
		connection,
		request->op.seek.fd,
		request->op.seek.offset,
		request->op.seek.whence
	);

	/* Operation failed. */
	if (ret < 0)
		return (ret);

	response->op.ret.offset = ret;

	return (0);
}

/*============================================================================*
 * do_vfs_server_write()                                                      *
 *============================================================================*/

/**
 * @brief Handles an write request.
 *
 * @param request  Target request.
 * @param response Response.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @author Pedro Henrique Penna
 */
static int do_vfs_server_write(
	const struct vfs_message *request,
	struct vfs_message *response
)
{
	ssize_t ret;
	const int port = request->header.mailbox_port;
	const nanvix_pid_t pid = request->header.source;
	const int connection = lookup(pid, port);

	/* Invalid write size. */
	if ((request->op.write.n == 0) || (request->op.write.n > NANVIX_FS_BLOCK_SIZE))
		return (-EINVAL);

	/* XXX: forward parameter checking to lower level function. */

	/* Allow remote write. */
	uassert(
		kportal_allow(
			server.inportal,
			request->header.source,
			request->header.portal_port
		) == 0
	);

	/* Read data in. */
	uassert(
		kportal_read(
			server.inportal,
			buffer,
			request->op.write.n
		) == (ssize_t) request->op.write.n
	);

	ret = vfs_write(
		connection,
		request->op.write.fd,
		buffer,
		request->op.write.n
	);

	/* Operation failed. */
	if (ret < 0)
		return (ret);

	response->op.ret.count = ret;

	return (0);
}

/*============================================================================*
 * do_vfs_server_read()                                                       *
 *============================================================================*/

/**
 * @brief Handles an read request.
 *
 * @param request  Target request.
 * @param response Response.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @author Pedro Henrique Penna
 */
static int do_vfs_server_read(
	const struct vfs_message *request,
	struct vfs_message *response
)
{
	ssize_t ret;
	int outportal;
	int outbox;
	struct vfs_message msg;
	const int port = request->header.mailbox_port;
	const nanvix_pid_t pid = request->header.source;
	const int connection = lookup(pid, port);

	/* Invalid read size. */
	if ((request->op.read.n == 0) || (request->op.read.n > NANVIX_FS_BLOCK_SIZE))
		return (-EINVAL);

	/* XXX: forward parameter checking to lower level function. */

	uassert((
		outbox = kmailbox_open(
			request->header.source,
			request->header.mailbox_port
		)) >= 0
	);

	/* Open portal to remote. */
	uassert((outportal =
		kportal_open(
			knode_get_num(),
			request->header.source,
			request->header.portal_port)
		) >= 0
	);

	/* Build operation header. */
	message_header_build2(
		&msg.header,
		VFS_ACK,
		kcomm_get_port(outportal, COMM_TYPE_PORTAL)
	);

	/* Send acknowledge. */
	uassert(
		kmailbox_write(outbox,
			&msg,
			sizeof(struct vfs_message)
		) == sizeof(struct vfs_message)
	);

	/* Write to remote. */
	uassert(
		kportal_write(
			outportal,
			buffer,
			request->op.read.n
		) == (ssize_t) request->op.read.n
	);

	/* House keeping. */
	uassert(kportal_close(outportal) == 0);
	uassert(kmailbox_close(outbox) == 0);

	ret = vfs_read(
		connection,
		request->op.read.fd,
		buffer,
		request->op.read.n
	);

	/* Operation failed. */
	if (ret < 0)
		return (ret);

	response->op.ret.count = ret;

	return (0);
}



/*============================================================================*
 * vfs_loop()                                                                 *
 *============================================================================*/

/**
 * @brief Handles file system region requests.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_vfs_server_loop(void)
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
				ret = do_vfs_server_open(&request, &response);
				reply = 1;
				break;

			case VFS_UNLINK:
				ret = do_vfs_server_unlink(&request);
				reply = 1;
				break;

			case VFS_CLOSE:
				ret = do_vfs_server_close(&request);
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
				ret = do_vfs_server_stat(&request, &response);
				break;

			case VFS_READ:
				ret = do_vfs_server_read(&request, &response);
				reply = 1;
				break;

			case VFS_WRITE:
				ret = do_vfs_server_write(&request, &response);
				reply = 1;
				break;

			case VFS_SEEK:
				reply = 1;
				ret = do_vfs_server_seek(&request, &response);
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

#ifndef __SUPPRESS_TESTS
	vfs_test();
#endif

	return (0);
}

/*============================================================================*
 * do_vfs_server_startup()                                                    *
 *============================================================================*/

/**
 * @brief Initializes the file system server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_vfs_server_startup(struct nanvix_semaphore *lock)
{
	int ret;

	uprintf("[nanvix][vfs] booting up server");

	server.nodenum = knode_get_num();
	server.inbox = stdinbox_get();
	server.inportal = stdinportal_get();

	/* Link name. */
	if ((ret = nanvix_name_link(server.nodenum, server.name)) < 0)
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
 * do_vfs_server_shutdown()                                                   *
 *============================================================================*/

/**
 * @brief Shutdowns the file system server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_vfs_server_shutdown(void)
{
	uprintf("[nanvix][vfs] shutting down server");
	vfs_shutdown();

	return (0);
}

/*============================================================================*
 * do_vfs_server_server()                                                     *
 *============================================================================*/

/**
 * @brief Virtual Files System server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_vfs_server_server(struct nanvix_semaphore *lock)
{
	int ret;

	if ((ret = do_vfs_server_startup(lock)) < 0)
	{
		uprintf("[nanvix][vfs] failed to startup server!");
		goto error;
	}

	if ((ret = do_vfs_server_loop()) < 0)
	{
		uprintf("[nanvix][vfs] failed to launch server!");
		goto error;
	}

	if ((ret = do_vfs_server_shutdown()) < 0)
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
	uassert(do_vfs_server_server(lock) == 0);

	return (0);
}
