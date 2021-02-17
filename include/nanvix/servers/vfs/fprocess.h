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

#ifndef NANVIX_SERVERS_VFS_FPROCESS_H_
#define NANVIX_SERVERS_VFS_FPROCESS_H_

	#ifndef __VFS_SERVER
	#error "do not include this file"
	#endif

	/* Must come first. */
	#define __NEED_LIMITS_FS

	#include <nanvix/limits/fs.h>

	/**
	 * @brief (File System) Process
	 */
	struct fprocess
	{
		int errcode;                          /**< Error Code        */
		mode_t umask;                   /**< Default umask     */
		struct inode *pwd;                    /**< Working Directory */
		struct inode *root;                   /**< Root Directory    */
		struct file *ofiles[NANVIX_OPEN_MAX]; /**< Opened Files      */
	};

	/**
	 * @brief Initialize the table of file system processes.
	 */
	extern void fprocess_init(void);

	/**
	 * @brief Launches a file system process.
	 *
	 * @param connection Target connection
	 *
	 * @returns Upon sucessful completion, zero is returned. Upon failure, a
	 * negative error code is returned instead.
	 */
	extern int fprocess_launch(int connection);

	/**
	 * @brief Current Process
	 */
	extern struct fprocess *curr_proc;

#endif /* NANVIX_SERVERS_VFS_FPROCESS_H_*/

