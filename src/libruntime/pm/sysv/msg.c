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
#include <posix/sys/types.h>

/*============================================================================*
 * __nanvix_msg_get()                                                         *
 *============================================================================*/

/**
 * The __do_nanvix_msg_get() function gets a message key that matches
 * the @p key parameter. The @p mspflg parameter specifies additional
 * actions to take during this operation. Upon successful completion,
 * zero is returned.
 *
 * @author Pedro Henrique Penna
 */
static int __do_nanvix_msg_get(key_t key, int msgflg)
{
	struct sysv_message msg;

	/* Client not initialized. */
	if (!__nanvix_sysv_is_initialized())
		return (-EAGAIN);

	/* Build message. */
	message_header_build(&msg.header, SYSV_MSG_GET);
	msg.payload.msg.get.key = key;
	msg.payload.msg.get.msgflg = msgflg;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			__nanvix_sysv_outbox(),
			&msg,
			sizeof(struct sysv_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct sysv_message)
		) == sizeof(struct sysv_message)
	);

	return (msg.payload.ret.status);
}

/**
 * @see __do_nanvix_msg_get().
 */
int __nanvix_msg_get(key_t key, int msgflg)
{
	/* TODO: check parameters. */

	return (__do_nanvix_msg_get(key, msgflg));
}

/*============================================================================*
 * __nanvix_msg_close()                                                       *
 *============================================================================*/

/**
 * The __do_nanvix_msg_close() function closes the message queue that is
 * identified by @p msgid. Upon successful completion, zero is returned.
 *
 * @author Pedro Henrique Penna
 */
int __do_nanvix_msg_close(int msgid)
{
	struct sysv_message msg;

	/* Client not initialized. */
	if (!__nanvix_sysv_is_initialized())
		return (-EAGAIN);

	/* Build message. */
	message_header_build(&msg.header, SYSV_MSG_CLOSE);
	msg.payload.msg.close.msgid = msgid;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			__nanvix_sysv_outbox(),
			&msg,
			sizeof(struct sysv_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct sysv_message)
		) == sizeof(struct sysv_message)
	);

	return (msg.payload.ret.status);
}

/**
 * @see __do_nanvix_msg_close().
 */
int __nanvix_msg_close(int msgid)
{
	/* Invalid ID for message queue. */
	if (!msgid_is_valid(msgid))
		return (-EINVAL);

	return (__do_nanvix_msg_close(msgid));
}

/*============================================================================*
 * __nanvix_msg_send()                                                        *
 *============================================================================*/

/**
 * The __do_nanvix_msg_send() function places the message pointed to by
 * @p msgp in the message queue that is identified by @p msgid. The
 * @p msgsz parameter gives the size in bytes of the message and @p
 * msgflg provides extra actions to take during the operation. Upon
 * sussceful completion, zero is returned.
 *
 * @author Pedro Henrique Penna
 */
int __do_nanvix_msg_send(
	int msgid,
	const void *msgp,
	size_t msgsz,
	int msgflg
)
{
	struct sysv_message msg;

	/* Client not initialized. */
	if (!__nanvix_sysv_is_initialized())
		return (-EAGAIN);

	/* Build message. */
	message_header_build2(
		&msg.header,
		SYSV_MSG_SEND,
		__nanvix_sysv_outportal()
	);
	msg.payload.msg.send.msgid = msgid;
	msg.payload.msg.send.msgsz = msgsz;
	msg.payload.msg.send.msgflg = msgflg;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			__nanvix_sysv_outbox(),
			&msg,
			sizeof(struct sysv_message)
		) == 0
	);

	/* Send data. */
	uassert(
		nanvix_portal_write(
			__nanvix_sysv_outportal(),
			msgp,
			msgsz	
		) == (ssize_t) msgsz 
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct sysv_message)
		) == sizeof(struct sysv_message)
	);

	return (msg.payload.ret.status);
}

/**
 * @see __do_nanvix_msg_send().
 */
int __nanvix_msg_send(int msgid, const void *msgp, size_t msgsz, int msgflg)
{
	/* Invalid ID for message queue. */
	if (!msgid_is_valid(msgid))
		return (-EINVAL);

	/* Invalid message. */
	if (msgp == NULL)
		return (-EINVAL);

	/* Invalid message size. */
	if (msgsz != NANVIX_MSG_SIZE_MAX)
		return (-EINVAL);

	/* TODO: check parameters. */

	return (__do_nanvix_msg_send(msgid, msgp, msgsz, msgflg));
}

/*============================================================================*
 * __nanvix_msg_receive()                                                     *
 *============================================================================*/

/**
 * The __do_nanvix_msg_receive() function retrives a message, from the
 * message queue @p msgid, that matches the specified type @p msgtyp and
 * places it in the location pointed to by @p msgp. The @p msgsz
 * parameter gives the size in bytes of the message and @p msgflg
 * provides extra actions to take during the operation. Upon sussceful
 * completion, zero is returned.
 *
 * @author Pedro Henrique Penna
 */
int __do_nanvix_msg_receive(
	int msgid,
	void *msgp,
	size_t msgsz,
	long msgtyp,
	int msgflg
)
{
	struct sysv_message msg;

	/* Client not initialized. */
	if (!__nanvix_sysv_is_initialized())
		return (-EAGAIN);

	/* Build message. */
	message_header_build(&msg.header, SYSV_MSG_RECEIVE);
	msg.payload.msg.receive.msgid = msgid;
	msg.payload.msg.receive.msgsz = msgsz;
	msg.payload.msg.receive.msgtyp = msgtyp;
	msg.payload.msg.receive.msgflg = msgflg;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			__nanvix_sysv_outbox(),
			&msg,
			sizeof(struct sysv_message)
		) == 0
	);

	/* Wait acknowledge. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct sysv_message)
		) == sizeof(struct sysv_message)
	);

	/* Operation not permitted*/
	if (msg.header.opcode != SYSV_ACK)
		return (msg.payload.ret.status);

	/* Receive data. */
	uassert(
		kportal_allow(
			stdinportal_get(),
			SYSV_SERVER_NODE,
			msg.header.portal_port
		) == 0
	);
	uassert(
		kportal_read(
			stdinportal_get(),
			msgp,
			msgsz
		) == (ssize_t) msgsz 
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct sysv_message)
		) == sizeof(struct sysv_message)
	);

	return (msg.payload.ret.status);
}

/**
 * @see __do_nanvix_msg_receive().
 */
int __nanvix_msg_receive(
	int msgid,
	void *msgp,
	size_t msgsz,
	long msgtyp,
	int msgflg
)
{
	/* Invalid ID for message queue. */
	if (!msgid_is_valid(msgid))
		return (-EINVAL);

	/* Invalid message. */
	if (msgp == NULL)
		return (-EINVAL);

	/* Invalid message size. */
	if (msgsz != NANVIX_MSG_SIZE_MAX)
		return (-EINVAL);

	/* TODO: check parameters. */

	return (__do_nanvix_msg_receive(msgid, msgp, msgsz, msgtyp, msgflg));
}
