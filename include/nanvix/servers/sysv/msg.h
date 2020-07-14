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

#ifndef NANVIX_SERVERS_SYSV_MSG_H_
#define NANVIX_SERVERS_SYSV_MSG_H_

	/* Must come first. */
	#define __NEED_LIMITS_PM

	#include <nanvix/limits/pm.h>
	#include <posix/sys/types.h>

	/**
	 * @name Types of Messages
	 */
	/**@{*/
	#define SYSV_MSG_GET      (1 << 2) /**< Get Message Queue       */
	#define SYSV_MSG_CLOSE    (2 << 2) /**< Close Message Queue     */
	#define SYSV_MSG_SEND     (3 << 2) /**< Send a Message          */
	#define SYSV_MSG_RECEIVE  (4 << 2) /**< Receive a Message       */
	/**@}*/

	/**
	 * @brief Payload for Message Queue Service
	 */
	union msg_payload
	{
		/**
		 * @brief Get Message
		 */
		struct
		{
			key_t key;  /**< Key  */
			int msgflg; /**< Flag */
		} get;

		/**
		 * @brief Close Message
		 */
		struct
		{
			int msgid; /**< ID */
		} close;

		/**
		 * @brief Send Message
		 */
		struct
		{
			int msgid;    /**< ID   */
			size_t msgsz; /**< Size */
			int msgflg;   /**< Flag */
		} send;

		/**
		 * @brief Receive Message
		 */
		struct
		{
			int msgid;    /**< ID   */
			size_t msgsz; /**< Size */
			long msgtyp;  /**< TYpe */
			int msgflg;   /**< Flag */
		} receive;
	};

	/**
	 * @brief Asserts if a message queue is valid.
	 *
	 * @param x Target messaque queue.
	 *
	 * @returns Non zero if the target message queue is valid and zero
	 * otherwise.
	 */
	#define msgid_is_valid(x) (((x) >= 0) && ((x) < NANVIX_MSG_MAX))

#ifdef __SYSV_SERVER

	/**
	 * @brief Opaque pointer to message buffer.
	 */
	typedef struct msgbuf *msgbuf_t;

	/**
	 * @broef Initializes message buffers.
	 */
	extern void msgbuf_init(void);

	/**
	 * @brief Allocates a message buffer.
	 *
	 * @returns Upon successful completion, a reference to a newly
	 * allocated message buffer is returned. Upon faillure, a negative
	 * error code is returned instead.
	 */
	extern msgbuf_t msgbuf_alloc(void);

	/**
	 * @brief Releases a message buffer.
	 *
	 * @param buf Target message buffer.
	 */
	extern void msgbuf_free(msgbuf_t buf);

	/**
	 * @brief Retrieves an object from a message buffer.
	 *
	 * @param buf Target message buffer.
	 * @param obj Store location for retrieved object.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int msgbuf_get(msgbuf_t buf, void **obj);

	/**
	 * @brief Places an object in a message buffer.
	 *
	 * @param buf Target message bufer.
	 * @param obj Target object.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int msgbuf_put(msgbuf_t buf, void **obj);

	/**
	 * @brief Initializes message queues.
	 */
	extern void do_msg_init(void);

	/**
	 * @brief Gets a message queue identifier.
	 *
	 * @param key    Key of the target message queue.
	 * @param msgflg Flags.
	 *
	 * @returns Upon successful completion, the identifier of the target
	 * message queue is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern int do_msg_get(key_t key, int msgflg);

	/**
	 * @brief Closes a message queue.
	 *
	 * @param msgid Identifier of target target message queue.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int do_msg_close(int msgid);

	/**
	 * @brief Places a message in a message queue.
	 *
	 * @param msgid  Identifier of target target message queue.
	 * @param msgp   Pointer to target message.
	 * @param msgsz  Size of the message.
	 * @param msgflg Flags.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int do_msg_send(
		int msgid,
		void **msgp,
		size_t msgsz,
		int msgflg
	);

	/**
	 * @brief Retrieves a message from a message queue.
	 *
	 * @param msgid  Identifier of target target message queue.
	 * @param msgp   Store location for message.
	 * @param msgsz  Size of the message.
	 * @param msgflg Flags.
	 * @param msgtyp Type of the message.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int do_msg_receive(
		int msgid,
		void **msgp,
		size_t msgsz,
		long msgtyp,
		int msgflg
	);

#endif /* __SYSV_SERVER */

#endif /* NANVIX_SERVERS_SYSV_MSG_H_ */
