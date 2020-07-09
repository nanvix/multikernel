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
 * copies or substantail portions of the Software.
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
#define __SYSV_SERVER

#include <nanvix/servers/sysv.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>


/**
 * @brief Size of message queue buffer.
 */
#define MSGBUF_SIZE (NANVIX_MSG_LENGTH_MAX*NANVIX_MSG_SIZE_MAX)

/**
 * @brief Message Buffer
 */
struct msgbuf
{
	size_t len;             /**< Current Length              */
	size_t size;            /**< Size of Messages (in bytes) */
	size_t head;            /**< First Message               */
	size_t tail;            /**< First Slot                  */
	char data[MSGBUF_SIZE]; /**< Data                        */
} buffers[NANVIX_MSG_MAX];

/*============================================================================*
 * msgbuf_alloc()                                                             *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
struct msgbuf *msgbuf_alloc(void)
{
	/* Find a free message buffer. */
	for (int i = 0; i < NANVIX_MSG_MAX; i++)
	{
		/* Found. */
		if (buffers[i].size == 0)
		{
			buffers[i].head = 0;
			buffers[i].tail = 0;
			buffers[i].size = NANVIX_MSG_SIZE_MAX;
			umemset(buffers[i].data, 0, MSGBUF_SIZE);
			return (&buffers[i]);
		}
	}

	return (NULL);
}

/*============================================================================*
 * msgbuf_release()                                                           *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
void msgbuf_free(struct msgbuf *buf)
{
	buf->size = 0;
}

/*============================================================================*
 * msgbuf_put()                                                               *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int msgbuf_put(struct msgbuf *buf, void **obj)
{
	/* Message buffer is full. */
	if (buf->len == NANVIX_MSG_LENGTH_MAX)
		return (-ENOSPC);

	buf->len++;
	*obj = &buf->data[buf->tail];
	buf->tail = (buf->tail + buf->size)%MSGBUF_SIZE;

	return (0);
}

/*============================================================================*
 * msgbuf_get()                                                               *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int msgbuf_get(struct msgbuf *buf, void **obj)
{
	/* Message buffer is full. */
	if (buf->len == 0)
		return (-ENOMSG);

	buf->len--;
	*obj = &buf->data[buf->head];
	buf->head = (buf->head + buf->size)%MSGBUF_SIZE;

	return (0);
}

/*============================================================================*
 * msgbuf_init()                                                              *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
void msgbuf_init(void)
{
	/* Initailize table of message buffers. */
	for (int i = 0; i < NANVIX_MSG_MAX; i++)
		buffers[i].size = 0, buffers[i].len = 0;
}
