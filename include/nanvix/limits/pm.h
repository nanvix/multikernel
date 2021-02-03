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

#ifndef NANVIX_LIMITS_PM_H_
#define NANVIX_LIMITS_PM_H_

	#ifndef __NEED_LIMITS_PM
	#error "do not include this file"
	#endif

	#include <nanvix/sys/portal.h>
	#include <nanvix/sys/mailbox.h>

	/**
	 * @brief Maximum number of processes.
	 */
	#define NANVIX_PROC_MAX PROCESSOR_CCLUSTERS_NUM

	/**
	 * @brief Maximum number of names for processes.
	 */
	#define NANVIX_PNAME_MAX 256

	/**
	 * @brief Maximum number of process groups
	 */
	#define NANVIX_GROUP_MAX 32

	/**
	 * @name Limits on Naming Service
	 */
	/**@{*/
	#define NANVIX_PROC_NAME_MAX 64 /**< Maximum length of a process name. */
	/**@}*/

	/**
	 * @name Limits on Named Mailboxes
	 */
	/**@{*/
	#define NANVIX_MAILBOX_MESSAGE_SIZE KMAILBOX_MESSAGE_SIZE /**< Maximum size for a mailbox message.             */
	#define NANVIX_MAILBOX_MAX          KMAILBOX_MAX          /**< Maximum number of mailboxes that can be opened. */
	/**@}*/

	/**
	 * @name Limits on Named Portals
	 */
	/**@{*/
	#define NANVIX_PORTAL_MAX KPORTAL_MAX /**< Maximum number of portals that can be opened. */

	/**
	 * @brief Maximum number of active connections in a server.
	 */
	#define NANVIX_CONNECTIONS_MAX (NANVIX_PROC_MAX * 4)

	/**
	 * @brief Defines the base for the general purpose ports range for communicators.
	 *
	 * @note It's the first not reserved port for standard structures of the IKC module.
	 *
	 * @note Just a workaround. Redefine to at least 16 when possible.
	 */
	#define NANVIX_GENERAL_PORTS_BASE 13

/*============================================================================*
 * System V Service                                                           *
 *============================================================================*/

	/**
	 * @brief Maximum number of message queues.
	 */
	#define NANVIX_MSG_MAX 16

	/**
	 * @brief Maximum length for a message queue.
	 */
	#define NANVIX_MSG_LENGTH_MAX 16

	/**
	 * @brief Maximum size for a message.
	 */
	#define NANVIX_MSG_SIZE_MAX 256

	/**
	 * @brief Maximum number of semaphores.
	 */
	#define NANVIX_SEM_MAX 64

	/**
	 * @name Limits on Shared Memory Regions
	 */
	/**@{*/
	#define NANVIX_SHM_MAX                128  /**< Maximum number of shared memory regions.        */
	#define NANVIX_SHM_OPEN_MAX             8  /**< Maximum number of opened shared memory regions. */
	#define NANVIX_SHM_NAME_MAX            64  /**< Maximum length for a shared memory region name. */
	#define NANVIX_SHM_SIZE_MAX       (4*1024) /**< Mamximum size for a shared memory region.       */
	#define NANVIX_SHM_MAP_SIZE_MAX  (64*1024) /**< Maximum mapping size (in bytes).                */
	/**@}*/

#endif /* NANVIX_LIMITS_PM_H_ */
