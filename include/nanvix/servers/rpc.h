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

#ifndef NANVIX_SERVERS_RPC_H_
#define NANVIX_SERVERS_RPC_H_

	#ifndef __NEED_RPC_SERVER
	#error "do not include this file"
	#endif

	/* Must come first. */
	#define __NEED_LIMITS_PM

	#include <nanvix/servers/message.h>
	#include <nanvix/limits/pm.h>
	#include <nanvix/ulib.h>
	#include <posix/stdint.h>
	#include <posix/errno.h>

#if __NANVIX_USE_TASKS

	/**
	 * @name Operation types for Name Server.
	 */
	/**@{*/
	#define RPC_NORMAL  0 /**< Resquest an ACK from acceptance.        */
	#define RPC_ONE_WAY 1 /**< Do not resquest an ACK from acceptance. */
	/**@}*/

	/**
	 * @brief Size of the RPC message.
	 */
	#define RPC_MESSAGE_SIZE (sizeof(struct rpc_message))

	/**
	 * @brief Maximum number of RPCs.
	 */
	#define RPC_MAX (16)

	/**
	 * @brief RPC function.
	 */
	typedef int (* rpc_fn)(
		int nodenum,
		int mailbox_port,
		int portal_port,
		word_t arg0,
		word_t arg1,
		word_t arg2,
		word_t arg3,
		word_t arg4,
		word_t arg5
	);

	/**
	 * @brief RPC message.
	 */
	struct rpc_message
	{
		/**
		 * @brief Message header.
		 */
		message_header header; /**< Message header.     */

		/**
		 * @name Task attributes.
		 */
		/**@{*/
		int rid;               /**< RPC Identification. */
		struct task_args args; /**< RPC arguments.      */
		/**@}*/
	};

	/**
	 * @brief RPC structure.
	 */
	struct rpc
	{
		int rid;                /**< RPC Identification.              */
		rpc_fn request;         /**< Function that will be requested. */
		rpc_fn response;        /**< Function to gets the response.   */
		struct task task;       /**< RPC arguments.                   */
		struct rpc_message msg; /**< Request message.                 */
	};

#endif /* __NANVIX_USE_TASKS */

#endif /* NANVIX_SERVERS_RPC_H_ */
