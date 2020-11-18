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

#ifndef NANVIX_RUNTIME_PM_RPC_H_
#define NANVIX_RUNTIME_PM_RPC_H_

	#ifndef __NEED_RPC_SERVICE
	#define "do not include this file"
	#endif

	#include <nanvix/kernel/config.h>

	/**
	 * @brief Initializes the Name Service client.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
#if __NANVIX_USE_TASKS
	extern int __nanvix_rpc_setup(void);
#else
	static int __nanvix_rpc_setup(void)
	{
		return (0);
	}
#endif

	/**
	 * @brief Shuts down the Name Service client.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
#if __NANVIX_USE_TASKS
	extern int __nanvix_rpc_cleanup(void);
#else
	static int __nanvix_rpc_cleanup(void)
	{
		return (0);
	}
#endif

#if __NANVIX_USE_TASKS

	/**
	 * @name Operation types for Name Server.
	 */
	/**@{*/
	#define RPC_MAILBOX_PORT (0)
	#define RPC_PORTAL_PORT  (0)
	/**@}*/

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
	 * @brief Create an RPC.
	 *
	 * @param rid RPC identification.
	 * @param req Requested function.
	 * @param rsp Gets response function.
	 *
	 * @returns Upon successful completion 0 is returned.
	 * Upon failure, a negative error code is returned instead.
	 */
	extern int nanvix_rpc_create(int rid, rpc_fn request, rpc_fn response);

	/**
	 * @brief Unlink an RPC.
	 *
	 * @param rid RPC identification.
	 *
	 * @returns Upon successful completion 0 is returned.
	 * Upon failure, a negative error code is returned instead.
	 */
	extern int nanvix_rpc_unlink(int rid);

	/**
	 * @brief Sends a RPC request to another processor.
	 *
	 * @param ... Args.
	 *
	 * @returns Upon successful 0. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern int nanvix_rpc_request(
		const char *name,
		int rid,
		int mode,
		word_t arg0,
		word_t arg1,
		word_t arg2,
		word_t arg3,
		word_t arg4,
		word_t arg5
	);

	/**
	 * @brief Gets the response of an RPC request to another processor.
	 *
	 * @param ... Args.
	 *
	 * @returns Upon successful 0. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern int nanvix_rpc_response(
		const char *name,
		int rid,
		word_t arg0,
		word_t arg1,
		word_t arg2,
		word_t arg3,
		word_t arg4,
		word_t arg5
	);

#endif /* __NANVIX_USE_TASKS */

#endif /* NANVIX_RUNTIME_PM_RPC_H_ */
