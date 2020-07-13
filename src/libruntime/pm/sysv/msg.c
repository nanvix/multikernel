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
 * @todo TODO: provide a detailed description for this function.
 */
static int __do_nanvix_msg_get(key_t key, int msgflg)
{
	struct sysv_message msg;

	/* Client not initialized. */
	if (!__nanvix_sysv_is_initialized())
		return (-EAGAIN);

	/* Build message. */
	message_header_build(&msg.header, SYSV_MSG_GET);
	msg.payload.msg.op.get.key = key;
	msg.payload.msg.op.get.msgflg = msgflg;

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
 * @todo TODO: provide a detailed description for this function.
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
 * @todo TODO: provide a detailed description for this function.
 */
int __do_nanvix_msg_close(int msgid)
{
	struct sysv_message msg;

	/* Client not initialized. */
	if (!__nanvix_sysv_is_initialized())
		return (-EAGAIN);

	/* Build message. */
	message_header_build(&msg.header, SYSV_MSG_CLOSE);
	msg.payload.msg.op.close.msgid = msgid;

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
 * @todo TODO: provide a detailed description for this function.
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
 * @todo TODO: provide a detailed description for this function.
 */
int __do_nanvix_msg_send(int msgid, const void *msgp, size_t msgsz, int msgflg)
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
	msg.payload.msg.op.send.msgid = msgid;
	msg.payload.msg.op.send.msgsz = msgsz;
	msg.payload.msg.op.send.msgflg = msgflg;

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
 * @todo TODO: provide a detailed description for this function.
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
 * @todo TODO: provide a detailed description for this function.
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
	msg.payload.msg.op.receive.msgid = msgid;
	msg.payload.msg.op.receive.msgsz = msgsz;
	msg.payload.msg.op.receive.msgtyp = msgtyp;
	msg.payload.msg.op.receive.msgflg = msgflg;

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
 * @todo TODO: provide a detailed description for this function.
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
