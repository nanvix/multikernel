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

#ifndef NANVIX_CONFIG_H_
#define NANVIX_CONFIG_H_

	#if defined(__mppa256__)
	#include <nanvix/config/mppa256.h>
	#elif defined(__unix64__)
	#include <nanvix/config/unix64.h>
	#endif

/*============================================================================*
 * Process Management                                                         *
 *============================================================================*/

	/**
	 * @brief User ID of Root User
	 */
	#define NANVIX_ROOT_UID 0

	/**
	 * @brief User Group ID of Root User
	 */
	#define NANVIX_ROOT_GID 0

	/**
	 * @brief Check if Root User
	 */
	#define IS_SUPERUSER(x) ((x) == NANVIX_ROOT_UID)

/*============================================================================*
 * Virtual File System                                                        *
 *============================================================================*/

	/**
	 * @brief Size of the RAM Disk
	 */
	#define NANVIX_DISK_SIZE (64*1024)

	/**
	 * @brief Root Device
	 */
	#define NANVIX_ROOT_DEV 0

	/**
	 * @brief Number of File System Buffers
	 */
	#define NANVIX_FS_NR_BUFFERS 32

	/**
	 * @brief Number of inodes.
	 */
	#define NANVIX_NR_INODES 64

/*============================================================================*
 * Memory Management System                                                   *
 *============================================================================*/

	/**
	 * @name Name of Servers
	 */
	/**@{*/
	#define VFS_SERVER_NAME  "/vfs"  /**< Virtual File System  */
	#define SYSV_SERVER_NAME "/sysv" /**< System V Server      */
	/**@}*/

	/**
	 * @brief Port number of Page Cache Snooper
	 */
	#define NANVIX_SHM_SNOOPER_PORT_NUM 2

#endif /* NANVIX_CONFIG_H_ */
