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

#ifndef NANVIX_SERVERS_NAME_H_
#define NANVIX_SERVERS_NAME_H_

	#ifndef __NEED_NAME_SERVER
	#error "do not include this file"
	#endif

	/* Must come first. */
	#define __NEED_LIMITS_PM

	#include <nanvix/servers/message.h>
	#include <nanvix/limits/pm.h>
	#include <nanvix/ulib.h>
	#include <posix/stdint.h>
	#include <posix/errno.h>

	/**
	 * @brief Operation types for Name Server.
	 */
	/**@{*/
	#define NAME_EXIT    0  /**< Exit request.            */
	#define NAME_LOOKUP  1  /**< lookup a name.           */
	#define NAME_LINK    2  /**< Add a new name.          */
	#define NAME_UNLINK  3  /**< Remove a name.           */
	#define NAME_SUCCESS 4  /**< Success acknowledgement. */
	#define NAME_ALIVE   5  /**< Client alive.            */
	#define NAME_FAIL    6  /**< Failure acknowledgement. */
	#define NAME_GETPID  7  /**< Get process id           */
	#define NAME_GETPGID 8  /**< Get process group id     */
	#define NAME_SETPGID 9  /**< Set process group id     */
	#define NAME_SETPID  10 /**< Set process id           */
	/**@}*/

	/**
	 * @brief Name Server message.
	 */
	struct name_message
	{
		/**
		 * @brief Message header.
		 */
		message_header header;
		union
		{
			struct
			{
				char name[NANVIX_PROC_NAME_MAX]; /**< Portal name. */

			} lookup;

			struct
			{
				char name[NANVIX_PROC_NAME_MAX]; /**< Portal name. */
			} link;

			struct
			{
				char name[NANVIX_PROC_NAME_MAX]; /**< Portal name. */

			} unlink;

			struct
			{
				uint64_t timestamp; /**< Time stamp. */

			} heartbeat;

			struct
			{
				pid_t pid;  /**< Process id       */
				pid_t pgid; /**< Process group id */
			} setpgid;

			struct
			{
				pid_t pid;  /**< Process id */
			} getpgid;

			struct
			{

			} exit;

			struct
			{
				int nodenum; /**< NoC node.   */
				pid_t pid;   /**< Process id  */
				int errcode; /**< Error code. */
			} ret;
		} op;
	};

	/**
	 * @see node_is_valid()
	 */
	#define proc_is_valid(x) node_is_valid(x)

	/**
	 * @brief Asserts if a name is valid.
	 *
	 * @param name Target name.
	 *
	 * @returns Zero if the name pointed to by @p name is valid and a
	 * negative error code otherwise.
	 */
	static inline int nanvix_name_is_valid(const char *name)
	{
		/* Invalid name. */
		if (name == NULL)
			return (-EINVAL);

		/* Bad name. */
		if (!ustrcmp(name, ""))
			return (-EINVAL);

		/* Name too long. */
		if ((ustrlen(name) >= (NANVIX_PROC_NAME_MAX - 1)))
			return (-ENAMETOOLONG);

		return (0);
	}

#endif /* NANVIX_SERVERS_NAME_H_ */
