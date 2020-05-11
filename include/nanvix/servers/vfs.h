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

#if defined(__NEED_FS_VFS_SERVER) || defined(__VFS_SERVER)

	#include <nanvix/servers/message.h>

	/**
	 * @bried Virtual File System Operations
	 */
	/**@{*/
	#define VFS_EXIT    0  /**< Exit    */
	#define VFS_SUCCESS 1  /**< Success */
	#define VFS_FAIL    2  /**< Failure */
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

			/* Return Message */
			struct
			{
				int status;  /**< Status Code */
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

