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
#define __SYSV_SERVER

#include <nanvix/servers/connection.h>
#include <nanvix/servers/sysv.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/sys/semaphore.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/noc.h>
#include <nanvix/config.h>
#include <nanvix/ulib.h>

/**
 * @brief SYSV Server information.
 */
static struct
{
	int nodenum;  /**< Node number.  */
	int inbox;    /**< Input mailbox */
	int inportal; /**< Input Portal  */
	const char *name;
} server = {
	-1, -1, -1, SYSV_SERVER_NAME
};

/*============================================================================*
 * sysv_loop()                                                                 *
 *============================================================================*/

/**
 * @brief Handles System V region requests.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_sysv_server_loop(void)
{
	int shutdown = 0;

	while (!shutdown)
	{
		int outbox;
		int reply = 0;
		int ret = -ENOSYS;
		struct sysv_message request;
		struct sysv_message response;

		uassert(
			kmailbox_read(
				server.inbox,
				&request,
				sizeof(struct sysv_message)
			) == sizeof(struct sysv_message)
		);

		sysv_debug("sysv request source=%d port=%d opcode=%d",
			request.header.source,
			request.header.portal_port,
			request.header.opcode
		);

		/* TODO check for bad node number. */

		/* Handle request. */
		switch (request.header.opcode)
		{
			/* Exit. */
			case SYSV_EXIT:
				shutdown = 1;
				break;

			default:
				break;
		}

		/* No reply? */
		if (!reply)
			continue;

		response.payload.ret.status = ret;
		message_header_build(
			&response.header,
			(ret < 0) ? SYSV_FAIL : SYSV_SUCCESS
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
				sizeof(struct sysv_message
			)) == sizeof(struct sysv_message)
		);
		uassert(kmailbox_close(outbox) == 0);
	}

	return (0);
}

/*============================================================================*
 * do_sysv_server_startup()                                                    *
 *============================================================================*/

/**
 * @brief Initializes the System V server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_sysv_server_startup(struct nanvix_semaphore *lock)
{
	int ret;

	uprintf("[nanvix][sysv] booting up server");

	server.nodenum = knode_get_num();
	server.inbox = stdinbox_get();
	server.inportal = stdinportal_get();

	/* Link name. */
	if ((ret = nanvix_name_link(server.nodenum, server.name)) < 0)
		return (ret);

	connections_setup();

	uprintf("[nanvix][sysv] minix System V created");

	uprintf("[nanvix][sysv] server alive");
	uprintf("[nanvix][sysv] attached to node %d", server.nodenum);
	uprintf("[nanvix][sysv] listening to mailbox %d", server.inbox);
	uprintf("[nanvix][sysv] listening to portal %d", server.inportal);

	nanvix_semaphore_up(lock);

	return (0);
}

/*============================================================================*
 * do_sysv_server_shutdown()                                                   *
 *============================================================================*/

/**
 * @brief Shutdowns the System V server.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
static int do_sysv_server_shutdown(void)
{
	uprintf("[nanvix][sysv] shutting down server");

	return (0);
}

/*============================================================================*
 * do_sysv_server()                                                           *
 *============================================================================*/

/**
 * @brief System V server.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
static int do_sysv_server(struct nanvix_semaphore *lock)
{
	int ret;

	if ((ret = do_sysv_server_startup(lock)) < 0)
	{
		uprintf("[nanvix][sysv] failed to startup server!");
		goto error;
	}

	if ((ret = do_sysv_server_loop()) < 0)
	{
		uprintf("[nanvix][sysv] failed to launch server!");
		goto error;
	}

	if ((ret = do_sysv_server_shutdown()) < 0)
	{
		uprintf("[nanvix][sysv] failed to shutdown server!");
		goto error;
	}

	return (0);

error:
	return (ret);
}

/*============================================================================*
 * sysv_server()                                                              *
 *============================================================================*/

/**
 * @brief Handles System V requests.
 *
 * @returns Always returns zero.
 */
int sysv_server(struct nanvix_semaphore *lock)
{
	uassert(do_sysv_server(lock) == 0);

	return (0);
}
