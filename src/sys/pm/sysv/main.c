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


/* Import definitions. */
extern void msg_test(void);

/*============================================================================*
 * do_sysv_msg_get()                                                          *
 *============================================================================*/

/**
 * @brief Handles a message queue get request.
 *
 * @param request  Target request.
 * @param response Response.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @author Pedro Henrique Penna
 */
static int do_sysv_msg_get(
	const struct sysv_message *request,
	struct sysv_message *response
)
{
	int ret;
	const pid_t pid = request->header.source;
	const int connection = connect(pid);

	((void) connection);
	ret = do_msg_get(
		request->payload.msg.op.get.key,
		request->payload.msg.op.get.msgflg
	);

	/* Operation failed. */
	if (ret < 0)
	{
		disconnect(pid);
		return (ret);
	}

	response->payload.ret.msgid = ret;

	return (ret);
}

/*============================================================================*
 * do_sysv_msg_close()                                                        *
 *============================================================================*/

/**
 * @brief Handles a message queue close request.
 *
 * @param request  Target request.
 * @param response Response.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @author Pedro Henrique Penna
 */
static int do_sysv_msg_close(const struct sysv_message *request)
{
	int ret;
	const pid_t pid = request->header.source;

	ret = do_msg_close(request->payload.msg.op.close.msgid);

	/* Operation failed. */
	if (ret < 0)
		return (ret);

	disconnect(pid);

	return (ret);
}

/*============================================================================*
 * do_sysv_msg_send()                                                         *
 *============================================================================*/

/**
 * @brief Handles a message queue send request.
 *
 * @param request  Target request.
 * @param response Response.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @author Pedro Henrique Penna
 */
static int do_sysv_msg_send(const struct sysv_message *request)
{
	int ret;
	void *msgp;

	ret = do_msg_send(
		request->payload.msg.op.send.msgid,
		&msgp,
		request->payload.msg.op.send.msgsz,
		request->payload.msg.op.send.msgflg
	);

	/**
	 * Note that even though the operation may be failed, we guarantee
	 * the correct protocol. In the return message we inform wether or
	 * not the operation has succeeded.
	 */

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
			msgp,
			request->payload.msg.op.send.msgsz
		) == (ssize_t) request->payload.msg.op.send.msgsz
	);

	return (ret);
}

/*============================================================================*
 * do_sysv_msg_receive()                                                      *
 *============================================================================*/

/**
 * @brief Handles a message queue receive request.
 *
 * @param request  Target request.
 * @param response Response.
 *
 * @returns Upon successful completion, zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @author Pedro Henrique Penna
 */
static int do_sysv_msg_receive(const struct sysv_message *request)
{
	int ret;
	int outportal;
	int outbox;
	void *msgp;
	struct sysv_message msg;

	ret = do_msg_receive(
		request->payload.msg.op.receive.msgid,
		&msgp,
		request->payload.msg.op.receive.msgsz,
		request->payload.msg.op.receive.msgtyp,
		request->payload.msg.op.receive.msgflg
	);

	/* Operation failed. */
	if (ret < 0)
		return (ret);

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
		SYSV_ACK,
		kcomm_get_port(outportal, COMM_TYPE_PORTAL)
	);

	/* Send acknowledge. */
	uassert(
		kmailbox_write(outbox,
			&msg,
			sizeof(struct sysv_message)
		) == sizeof(struct sysv_message)
	);

	/* Write to remote. */
	uassert(
		kportal_write(
			outportal,
			msgp,
			request->payload.msg.op.receive.msgsz
		) == (ssize_t) request->payload.msg.op.receive.msgsz
	);

	/* House keeping. */
	uassert(kportal_close(outportal) == 0);
	uassert(kmailbox_close(outbox) == 0);

	return (ret);
}

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
			/* Get message queue. */
			case SYSV_MSG_GET:
				ret = do_sysv_msg_get(&request, &response);
				reply = 1;
				break;

			/* Close message queue. */
			case SYSV_MSG_CLOSE:
				ret = do_sysv_msg_close(&request);
				reply = 1;
				break;

			/* Send message. */
			case SYSV_MSG_SEND:
				ret = do_sysv_msg_send(&request);
				reply = 1;
				break;

			/* Receive message. */
			case SYSV_MSG_RECEIVE:
				ret = do_sysv_msg_receive(&request);
				reply = 1;
				break;

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

#ifndef __SUPPRESS_TESTS
	uprintf("[nanvix][sysv] running self-tests...");
	msg_test();
#endif

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
	do_msg_init();

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
