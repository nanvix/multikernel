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
#define __NEED_SYSV_SERVER
#define __SYSV_SERVICE

#include <nanvix/runtime/runtime.h>
#include <nanvix/config.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

#ifdef __NANVIX_HAS_SYSV_SERVER

/**
 * @brief Connection to SYSVem Server.
 */
static struct
{
	int initialized; /**< Is the connection initialized? */
	int outbox;      /**< Output mailbox for requests.   */
	int outportal;   /**< Output portal for data.        */
} server = {
	.initialized = 0,
	.outbox = -1,
	.outportal = -1
};

#endif

/*============================================================================*
 * __nanvix_sysv_is_initialized()                                             *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_sysv_is_initialized(void)
{
	return (server.initialized);
}

/*============================================================================*
 * __nanvix_sysv_outbox()                                                     *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_sysv_outbox(void)
{
	return (server.outbox);
}

/*============================================================================*
 * __nanvix_sysv_outportal()                                                  *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_sysv_outportal(void)
{
	return (server.outportal);
}

/*============================================================================*
 * nanvix_sysv_shutdown()                                                      *
 *===========================================================================*/

/**
 * The nanvix_sysv_shutdown() function issues the shutdown signal to the
 * System V server, thus causing the server to terminate.
 */
int nanvix_sysv_shutdown(void)
{
#ifdef __NANVIX_HAS_SYSV_SERVER

	struct sysv_message msg;

	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Build operation header. */
	message_header_build(&msg.header, SYSV_EXIT);

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct sysv_message)
		) == 0
	);

#endif

	return (0);
}

/*============================================================================*
 * __nanvix_sysv_setup()                                                       *
 *============================================================================*/

/**
 * The __nanvhx_sysv_setup() function initializes the local daemon for
 * the System V Service.
 */
int __nanvix_sysv_setup(void)
{
#ifdef __NANVIX_HAS_SYSV_SERVER

	/* Nothing to do.  */
	if (server.initialized)
		return (0);

	/* Open output mailbox */
	if ((server.outbox = nanvix_mailbox_open(SYSV_SERVER_NAME, SYSV_SERVER_PORT_NUM)) < 0)
	{
		uprintf("[nanvix][sysv] cannot open outbox to server");
		return (server.outbox);
	}

	/* Open underlying IPC connectors. */
	if ((server.outportal = nanvix_portal_open(SYSV_SERVER_NAME, SYSV_SERVER_PORT_NUM)) < 0)
	{
		uprintf("[nanvix][sysv] cannot open outportal to server");
		return (server.outportal);
	}

	server.initialized = true;
	uprintf("[nanvix][sysv] connection with server established");

#endif

	return (0);
}

/*============================================================================*
 * __nanvix_sysv_cleanup()                                                    *
 *============================================================================*/

/**
 * The __nanvix_sysv_cleanup() function shutdowns the local daemon for
 * the System V Service.
 */
int __nanvix_sysv_cleanup(void)
{
#ifdef __NANVIX_HAS_SYSV_SERVER

	int ret;

	/* Nothing to do. */
	if (!server.initialized)
		return (0);

	/* Close output mailbox. */
	if ((ret = nanvix_mailbox_close(server.outbox)) < 0)
	{
		uprintf("[nanvix][sysv] cannot close outbox to server");
		return (ret);
	}

	/* Close underlying IPC connectors. */
	if (nanvix_portal_close(server.outportal) < 0)
	{
		uprintf("[nanvix][sysv] cannot close outportal to server");
		return (-EAGAIN);
	}

	server.initialized = 0;

#endif

	return (0);
}

