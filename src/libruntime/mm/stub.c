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

#define __NEED_RMEM_SERVICE

#include <nanvix/servers/rmem.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/runtime/mailbox.h>
#include <nanvix/runtime/portal.h>
#include <nanvix/sys/excp.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/mutex.h>
#include <nanvix/sys/noc.h>
#include <nanvix/config.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/stdbool.h>

/**
 * @brief Remote memory server connection.
 */
static struct
{
	int initialized; /**< Is the connection initialized? */
	int outbox;      /**< Output mailbox for requests.   */
	int outportal;   /**< Output portal for data.        */
} server[RMEM_SERVERS_NUM] = {
	[0 ... (RMEM_SERVERS_NUM - 1)] = { 0, -1, -1 }
};

/*============================================================================*
 * nanvix_rmem_alloc()                                                        *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
rpage_t nanvix_rmem_alloc(void)
{
	int serverid;
	static unsigned nallocs = 0;
	struct rmem_message msg;

	serverid = nallocs % RMEM_SERVERS_NUM;

	/* Client not initialized.  */
	if (!server[serverid].initialized)
		return (-EINVAL);

	/* Build operation header. */
	message_header_build(&msg.header, RMEM_ALLOC);

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server[serverid].outbox,
			&msg, sizeof(struct rmem_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct rmem_message)
		) == sizeof(struct rmem_message)
	);

	if (msg.errcode == RMEM_NULL)
		return RMEM_NULL;

	nallocs++;
	return (msg.blknum);
}

/*============================================================================*
 * nanvix_rmem_free()                                                         *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_rmem_free(rpage_t blknum)
{
	int serverid;
	struct rmem_message msg;

	/* Invalid block number. */
	if ((blknum == RMEM_NULL) || (RMEM_BLOCK_NUM(blknum) >= RMEM_NUM_BLOCKS))
		return (-EINVAL);

	serverid = RMEM_BLOCK_SERVER(blknum);

	/* Client not initialized.  */
	if (!server[serverid].initialized)
		return (-EINVAL);

	/* Build operation header. */
	message_header_build(&msg.header, RMEM_MEMFREE);
	msg.blknum = blknum;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server[serverid].outbox,
			&msg,
			sizeof(struct rmem_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct rmem_message)
		) == sizeof(struct rmem_message)
	);

	return (msg.errcode);
}

/*============================================================================*
 * nanvix_rmem_read()                                                         *
 *============================================================================*/

#ifndef __RMEM_USES_MAILBOX

/**
 * @todo TODO: Provide a detailed description for this function.
 */
size_t nanvix_rmem_read(rpage_t blknum, void *buf)
{
	int serverid;
	struct rmem_message msg;

	/* Invalid block number. */
	if ((blknum == RMEM_NULL) || (RMEM_BLOCK_NUM(blknum) >= RMEM_NUM_BLOCKS))
		return (0);

	/* Invalid buffer. */
	if (buf == NULL)
		return (0);

	serverid = RMEM_BLOCK_SERVER(blknum);

	/* Client not initialized.  */
	if (!server[serverid].initialized)
		return (0);

	/* Build operation header. */
	message_header_build(&msg.header, RMEM_READ);

	msg.blknum = blknum;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server[serverid].outbox,
			&msg,
			sizeof(struct rmem_message)
		) == 0
	);

	/* Wait acknowledge. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct rmem_message)
		) == sizeof(struct rmem_message)
	);
	uassert(msg.header.opcode == RMEM_ACK);

	/* Receive data. */
	uassert(
		kportal_allow(
			stdinportal_get(),
			rmem_servers[serverid].nodenum,
			msg.header.portal_port
		) == 0
	);
	uassert(
		kportal_read(
			stdinportal_get(),
			buf,
			RMEM_BLOCK_SIZE
		) == RMEM_BLOCK_SIZE
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct rmem_message)
		) == sizeof(struct rmem_message)
	);

	return ((msg.errcode < 0) ? 0 : RMEM_BLOCK_SIZE);
}

#else

/**
 * @todo TODO: Provide a detailed description for this function.
 */
size_t nanvix_rmem_read(rpage_t blknum, void *buf)
{
	int serverid;
	struct rmem_message msg;

	/* Invalid block number. */
	if ((blknum == RMEM_NULL) || (RMEM_BLOCK_NUM(blknum) >= RMEM_NUM_BLOCKS))
		return (0);

	/* Invalid buffer. */
	if (buf == NULL)
		return (0);

	serverid = RMEM_BLOCK_SERVER(blknum);

	/* Client not initialized.  */
	if (!server[serverid].initialized)
		return (0);

	/* Build operation header. */
	message_header_build(&msg.header, RMEM_READ);

	msg.blknum = blknum;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server[serverid].outbox,
			&msg,
			sizeof(struct rmem_message)
		) == 0
	);

	for (size_t i = 0; i < RMEM_BLOCK_SIZE; i += RMEM_PAYLOAD_SIZE)
	{
		/* Wait acknowledge. */
		uassert(
			kmailbox_read(
				stdinbox_get(),
				&msg,
				sizeof(struct rmem_message)
			) == sizeof(struct rmem_message)
		);
		uassert(msg.header.opcode == RMEM_ACK);

		umemcpy(&((char *)buf)[i], &msg.payload, RMEM_PAYLOAD_SIZE);

	}

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct rmem_message)
		) == sizeof(struct rmem_message)
	);

	return ((msg.errcode < 0) ? 0 : RMEM_BLOCK_SIZE);
}

#endif

/*============================================================================*
 * nanvix_rmem_write()                                                        *
 *============================================================================*/

#ifndef __RMEM_USES_MAILBOX

/**
 * @todo TODO: Provide a detailed description for this function.
 */
size_t nanvix_rmem_write(rpage_t blknum, const void *buf)
{
	int serverid;
	struct rmem_message msg;

	/* Invalid block number. */
	if ((blknum == RMEM_NULL) || (RMEM_BLOCK_NUM(blknum) >= RMEM_NUM_BLOCKS))
		return (0);

	/* Invalid buffer. */
	if (buf == NULL)
		return (0);

	serverid = RMEM_BLOCK_SERVER(blknum);

	/* Client not initialized.  */
	if (!server[serverid].initialized)
		return (0);

	/* Build operation header. */
	message_header_build2(
		&msg.header,
		RMEM_WRITE,
		nanvix_portal_get_port(server[serverid].outportal)
	);
	msg.blknum = blknum;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server[serverid].outbox,
			&msg, sizeof(struct rmem_message)
		) == 0
	);

	/* Send data. */
	uassert(
		nanvix_portal_write(
			server[serverid].outportal,
			buf,
			RMEM_BLOCK_SIZE
		) == RMEM_BLOCK_SIZE
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct rmem_message)
		) == sizeof(struct rmem_message)
	);

	return ((msg.errcode < 0) ? 0 : RMEM_BLOCK_SIZE);
}

#else

/**
 * @todo TODO: Provide a detailed description for this function.
 */
size_t nanvix_rmem_write(rpage_t blknum, const void *buf)
{
	int serverid;
	struct rmem_message msg;

	/* Invalid block number. */
	if ((blknum == RMEM_NULL) || (RMEM_BLOCK_NUM(blknum) >= RMEM_NUM_BLOCKS))
		return (0);

	/* Invalid buffer. */
	if (buf == NULL)
		return (0);

	serverid = RMEM_BLOCK_SERVER(blknum);

	/* Client not initialized.  */
	if (!server[serverid].initialized)
		return (0);

	for (size_t i = 0; i < RMEM_BLOCK_SIZE; i += RMEM_PAYLOAD_SIZE)
	{
		message_header_build2(
			&msg.header,
			RMEM_WRITE,
			nanvix_portal_get_port(server[serverid].outportal)
		);
		msg.blknum = blknum;
		msg.offset = i;

		umemcpy(&msg.payload, &((const char *)buf)[i], RMEM_PAYLOAD_SIZE);

		uassert(
			nanvix_mailbox_write(
				server[serverid].outbox,
				&msg, sizeof(struct rmem_message)
			) == 0
		);

		/* Receive reply. */
		uassert(
			kmailbox_read(
				stdinbox_get(),
				&msg,
				sizeof(struct rmem_message)
			) == sizeof(struct rmem_message)
		);
	}

	return ((msg.errcode < 0) ? 0 : RMEM_BLOCK_SIZE);
}

#endif

/*============================================================================*
 * nanvix_rmem_shutdown()                                                     *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_rmem_shutdown(void)
{
	struct rmem_message msg;

	/* Build operation header. */
	message_header_build(&msg.header, RMEM_EXIT);

	/* Broadcast shutdown signal. */
	for (int i = 0; i < RMEM_SERVERS_NUM; i++)
	{
		uassert(
			nanvix_mailbox_write(
				server[i].outbox,
				&msg, sizeof(struct rmem_message)
			) == 0
		);
	}

	return (0);
}

/*============================================================================*
 * nanvix_rmem_setup()                                                        *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_rmem_setup(void)
{
	/* Open connections to remote memory servers. */
	for (int i = 0; i < RMEM_SERVERS_NUM; i++)
	{
		/* Nothing to do.  */
		if (server[i].initialized)
			continue;

		/* Open output mailbox */
		if ((server[i].outbox = nanvix_mailbox_open(rmem_servers[i].name, rmem_servers[i].portnum)) < 0)
		{
			uprintf("[nanvix][rmem] cannot open outbox to server");
			return (server[i].outbox);
		}

		/* Open underlying IPC connectors. */
		if ((server[i].outportal = nanvix_portal_open(rmem_servers[i].name, rmem_servers[i].portnum)) < 0)
		{
			uprintf("[nanvix][rmem] cannot open outportal to server");
			return (server[i].outportal);
		}

		uprintf("[nanvix][rmem] connection with server established");
		server[i].initialized = 1;
	}

#if (CLUSTER_HAS_TLB_SHOOTDOWN)
	uassert(excp_ctrl(EXCEPTION_PAGE_FAULT, EXCP_ACTION_HANDLE) == 0);
#endif

	return (0);
}

/*============================================================================*
 * nanvix_rmem_cleanup()                                                      *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_rmem_cleanup(void)
{
#if (CLUSTER_HAS_TLB_SHOOTDOWN)
	uassert(excp_ctrl(EXCEPTION_PAGE_FAULT, EXCP_ACTION_IGNORE) == 0);
#endif

	/* Close connections to remote memory servers. */
	for (int i = 0; i < RMEM_SERVERS_NUM; i++)
	{
		/* Nothing to do.  */
		if (!server[i].initialized)
			continue;

		/* Close output mailbox */
		if (nanvix_mailbox_close(server[i].outbox) < 0)
		{
			uprintf("[nanvix][rmem] cannot close outbox to server");
			return (-EAGAIN);
		}

		/* Close underlying IPC connectors. */
		if (nanvix_portal_close(server[i].outportal) < 0)
		{
			uprintf("[nanvix][rmem] cannot close outportal to server");
			return (-EAGAIN);
		}

		server[i].initialized = 0;
	}

	return (0);
}
