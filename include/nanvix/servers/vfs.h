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

#ifndef NANVIX_SERVERS_VFS_H_
#define NANVIX_SERVERS_VFS_H_

	#include <nanvix/servers/vfs/const.h>
	#include <nanvix/servers/vfs/types.h>
	#include <posix/sys/stat.h>

#if defined(__NEED_FS_VFS_SERVER) || defined(__VFS_SERVER)

	#define __NEED_LIMITS_FS

	#include <nanvix/servers/message.h>
	#include <nanvix/limits/fs.h>

#ifdef __VFS_SERVER
	#include <nanvix/servers/vfs/bcache.h>
	#include <nanvix/servers/vfs/fs.h>
	#include <nanvix/servers/vfs/minix.h>
	#include <nanvix/servers/vfs/fprocess.h>
	#include <nanvix/servers/vfs/vfs.h>
#endif


	/**
	 * @bried Virtual File System Operations
	 */
	/**@{*/
	#define VFS_EXIT      0 /**< Exit        */
	#define VFS_SUCCESS   1 /**< Success     */
	#define VFS_FAIL      2 /**< Failure     */
	#define VFS_CREAT     3 /**< Create      */
	#define VFS_OPEN      4 /**< Open        */
	#define VFS_UNLINK    5 /**< Unlink      */
	#define VFS_CLOSE     6 /**< Close       */
	#define VFS_LINK      7 /**< Link        */
	#define VFS_TRUNCATE  8 /**< Truncate    */
	#define VFS_STAT      9 /**< Stat        */
	#define VFS_READ     10 /**< Read        */
	#define VFS_WRITE    11 /**< Write       */
	#define VFS_SEEK     12 /**< Seek        */
	#define VFS_ACK      13 /**< Acknowledge */
	/**@}*/

	/**
	 * @brief Shared Memory Region message.
	 */
	struct vfs_message
	{
		message_header header; /**< Message header.  */

		/* Operation-specific Fields */
		union
		{
			/**
			 * @brief Open
			 */
			struct
			{
				char filename[NANVIX_NAME_MAX]; /**< File Name  */
				int oflag;                      /**< Open Flags */
				mode_t mode;                    /**< Mode Flags */
			} open;

			/**
			 * @brief Stat
			 */
			struct
			{
				char filename[NANVIX_NAME_MAX]; /**< File Name  */
				struct nanvix_stat buf;         /**< Stats buf */
			} stat;

			/**
			 * @brief Close
			 */
			struct
			{
				int fd; /**< File Descriptor */
			} close;

			/**
			 * @brief Unlink
			 */
			struct
			{
				char filename[NANVIX_NAME_MAX]; /**< File path */
			} unlink;

			/**
			 * @brief Seek
			 */
			struct
			{
				int fd;       /**< File Descriptor */
				off_t offset; /**< File Offset     */
				int whence;   /**< Reposition Base */
			} seek;

			/**
			 * @brief Read
			 */
			struct
			{
				int fd;   /**< File Descriptor */
				size_t n; /**< Read            */
			} read;

			/**
			 * @brief Write
			 */
			struct
			{
				int fd;   /**< File Descriptor */
				size_t n; /**< Write           */
			} write;

			/* Return Message */
			struct
			{
				int fd;        /**< File Descriptor  */
				ssize_t count; /**< Read/Write Count */
				int status;    /**< Status Code      */
				off_t offset;  /**< File Offset      */
			} ret;
		} op;
	};

#ifdef __VFS_SERVER

	/**
	 * @brief Debug VFS Server?
	 */
	#define __DEBUG_VFS 0

	#if (__DEBUG_VFS)
	#define vfs_debug(fmt, ...) uprintf(fmt, __VA_ARGS__)
	#else
	#define vfs_debug(fmt, ...) { }
	#endif

#endif /*__VFS_SERVER */

#endif /* defined(__NEED_VFS_SERVER) || defined(__VFS_SERVER) */

#endif /* NANVIX_SERVERS_VFS_H_ */

