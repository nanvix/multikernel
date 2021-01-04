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

#ifndef NANVIX_SERVERS_VFS_VFS_H_
#define NANVIX_SERVERS_VFS_VFS_H_

	#ifndef __VFS_SERVER
	#error "do not include this file"
	#endif

	#include <posix/sys/types.h>

	/**
	 * @brief Initializes the virtual file system.
	 */
	extern void vfs_init(void);

	/**
	 * @brief Shutdowns the virtual file system.
	 */
	extern void vfs_shutdown(void);

	/**
	 * @brief Gets stats about a file
	 *
	 * @param connection Target connection.
	 * @param filename   Filename Name of the target file.
	 * @param buf        Stat struct to write stats to
	 *
	 * @returns Upon successful completion, returns 0
	 * Upon failure, a negative error code is returned instead.
	 */
	extern int vfs_stat(
		int connection,
		const char *filename,
		struct nanvix_stat *restrict buf
	);

	/**
	 * @brief Opens a file.
	 *
	 * @param connection Target connection.
	 * @param filename   Filename Name of the target file.
	 * @param oflag      Open flag.
	 * @param mode       Access mode.
	 *
	 * @returns Upon successful completion, the file descriptor of the
	 * opened file is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern int vfs_open(
		int connection,
		const char *filename,
		int oflag,
		mode_t mode
	);

	/**
	 * @brief Closes a file.
	 *
	 * @param connection Target connection.
	 * @param fd         Target file descriptor.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int vfs_close(int connection, int fd);

	/**
 	 * @brief Unlink a file from it's directory
 	 *
	 * @param connection Target connection.
 	 * @param filename: path of the file to be unlinked
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
 	 */
	extern int vfs_unlink(int connection, const char *filename);

	/**
	 * @brief Repositions the read/write pointer of a file.
	 *
	 * @param connection Target connection.
	 * @param fd         Target file descriptor.
	 * @param offset     Offset for read/write pointer.
	 * @param whence     Reposition base location.
	 *
	 * @returns Upon successful completion, the resulting offset
	 * location is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern off_t vfs_seek(
		int connection,
	   	int fd,
		off_t offset,
		int whence
	);

	/**
	 * @brief Reads data from a file.
	 *
	 * @param connection Target connection.
	 * @param fd         Target file descriptor.
	 * @param buf        Target buffer.
	 * @param n          Number of bytes to read.
	 *
	 * @returns Upon successful completion, the number of bytes
	 * successfully read is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	extern ssize_t vfs_read(
		int connection,
		int fd,
		void *buf,
		size_t n
	);

	/**
	 * @brief Writes data to a file.
	 *
	 * @param connection Target connection.
	 * @param fd         Target file descriptor.
	 * @param buf        Target buffer.
	 * @param n          Number of bytes to write.
	 *
	 * @returns Upon successful completion, the number of bytes
	 * successfully written is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	extern ssize_t vfs_write(
		int connection,
		int fd,
		void *buf,
		size_t n
	);

#endif /* NANVIX_SERVERS_VFS_VFS_H_*/
