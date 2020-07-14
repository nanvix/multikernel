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

#ifndef NANVIX_SERVERS_SYSV_H_
#define NANVIX_SERVERS_SYSV_H_

	/* Must come first. */
	#define __NEED_LIMITS_PM

	#include <nanvix/limits/pm.h>
	#include <nanvix/servers/sysv/sem.h>

#if defined(__NEED_SYSV_SERVER) || defined(__SYSV_SERVER)

	#include <nanvix/servers/sysv/msg.h>
	#include <nanvix/servers/message.h>

	/**
	 * @name Types of Messages
	 */
	/**@{*/
	#define SYSV_SUCCESS (0 << 0) /**< Success */
	#define SYSV_ACK     (1 << 0) /**< Fail    */
	#define SYSV_FAIL    (2 << 0) /**< Fail    */
	#define SYSV_EXIT    (3 << 0) /**< Exit    */
	/**@}*/

	/**
	 * @brief System V Message
	 */
	struct sysv_message
	{
		/**
		 * @brief Header
		 */
		message_header header;

		/**
		 * @brief Payload
		 */
		union
		{
			/**
			 * @brief Message Queue
			 */
			union msg_payload msg;

			/**
			 * @brief Semaphore
			 */
			union sem_payload sem;

			/**
			 * @brief Return Message
			 */
			struct
			{
				int ipcid;   /**< ID of IPC Structure */
				int status; /**< Status Code          */
			} ret;
		} payload;
	};

#ifdef __SYSV_SERVER

	/**
	 * @brief Debug System V Server?
	 */
	#define __DEBUG_SYSV 0

	#if (__DEBUG_SYSV)
	#define sysv_debug(fmt, ...) uprintf(fmt, __VA_ARGS__)
	#else
	#define sysv_debug(fmt, ...) { }
	#endif

#endif /* __SYSV_SERVER */

#endif /* __NEED_SYSV_SERVER || __SYSV_SERVER */

#endif /* NANVIX_SERVERS_SYSV_H_ */
