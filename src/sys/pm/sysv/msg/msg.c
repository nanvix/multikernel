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

/* Must come first.*/
#define __NEED_RESOURCE
#define __SYSV_SERVER

#include <nanvix/ulib.h>
#include <nanvix/hal/resource.h>
#include <nanvix/servers/sysv.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

/**
 * @brief Null Message.
 */
static char msgnull[NANVIX_MSG_SIZE_MAX];

/**
 * @brief Table of message queues.
 */
static struct msg
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource; /**< Generic resource information.  */

	pid_t owner;              /**< ID of owner process.  */
	key_t key;                /**< Key.                  */
	int refcount;             /**< Number of references. */
	mode_t mode;              /**< Access permissions.   */
	msgbuf_t buf;             /**< Message Buffer.       */
} mqueues[NANVIX_MSG_MAX];

/**
 * @brief Pool of message queues.
 */
static struct resource_pool pool = {
	mqueues, NANVIX_MSG_MAX , sizeof(struct msg)
};

/**
 * @todo TODO: provide a detailed description for this function.
 */
int do_msg_get(key_t key, int msgflg)
{
	int msgid;

	sysv_debug("do_msg_get() key=%d, msgflg=%x", key, msgflg);

	((void) msgflg);

	/* Search for key. */
	for (int i = 0; i < NANVIX_MSG_MAX; i++)
	{
		/* Skip invalid entries. */
		if (!resource_is_used(&mqueues[i].resource))
			continue;

		/* Found. */
		if (mqueues[i].key == key)
			goto found;
	}

	/* Allocate  message queue. */
	if ((msgid = resource_alloc(&pool)) < 0)
		return (-ENOMEM);

	mqueues[msgid].key = key;

found:

	mqueues[msgid].refcount++;
	return (msgid);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int do_msg_close(int msgid)
{
	sysv_debug("do_msg_close() msgid=%d", msgid);

	/* Invalid ID for message queue. */
	if (!msgid_is_valid(msgid))
		return (-EINVAL);

	/* Bad ID for message queue. */
	if (!resource_is_used(&mqueues[msgid].resource))
		return (-EINVAL);

	mqueues[msgid].refcount--;

	/* Release message queue. */
	if (mqueues[msgid].refcount == 0)
		resource_free(&pool, msgid);

	return (0);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int do_msg_send(int msgid, void **msgp, size_t msgsz, int msgflg)
{
	sysv_debug("do_msg_send() msgid=%d, msgsz=%d, msgflg=%x",
		msgid,
		msgsz,
		msgflg
	);

	/* Invalid ID for message queue. */
	if (!msgid_is_valid(msgid))
		goto error;

	/* Bad ID for message queue. */
	if (!resource_is_used(&mqueues[msgid].resource))
		goto error;

	/* Invalid message. */
	if (msgp == NULL)
		return (-EINVAL);

	/* Invalid message size. */
	if (msgsz != NANVIX_MSG_SIZE_MAX)
		goto error;

	((void) msgflg);

	return (msgbuf_put(mqueues[msgid].buf, msgp));

error:
	*msgp = msgnull;
	return (-EINVAL);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int do_msg_receive(int msgid, void **msgp, size_t msgsz, long msgtyp, int msgflg)
{
	sysv_debug("do_msg_receive() msgid=%d, msgsz=%d, msgtyp=%d, msgflg=%x",
		msgid,
		msgsz,
		msgtyp,
		msgflg
	);

	/* Invalid ID for message queue. */
	if (!msgid_is_valid(msgid))
		return (-EINVAL);

	/* Bad ID for message queue. */
	if (!resource_is_used(&mqueues[msgid].resource))
		return (-EINVAL);

	/* Invalid message. */
	if (msgp == NULL)
		return (-EINVAL);

	/* Invalid message size. */
	if (msgsz != NANVIX_MSG_SIZE_MAX)
		return (-EINVAL);

	((void) msgtyp);
	((void) msgflg);

	return (msgbuf_get (mqueues[msgid].buf, msgp));
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
void do_msg_init(void)
{
	msgbuf_init();

	/* Initialize table of message queues. */
	for (int i = 0; i < NANVIX_MSG_MAX; i++)
	{
		mqueues[i].refcount = 0;
		mqueues[i].buf = msgbuf_alloc();
		mqueues[i].resource = RESOURCE_INITIALIZER;
	}
}
