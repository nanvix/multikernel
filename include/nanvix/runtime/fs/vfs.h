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

#ifndef NANVIX_RUNTIME_FS_VFS_H_
#define NANVIX_RUNTIME_FS_VFS_H_

	#include <posix/sys/types.h>
	#include <posix/sys/stat.h>

	/**
	 * @brief Initializes the VFS Service.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_vfs_setup(void);

	/**
	 * @brief Shutdowns the VFS Service.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_vfs_cleanup(void);

	/**
	 * @brief Shutdowns the VFS Service.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_vfs_shutdown(void);

	/**
	 * @brief Gets File Stats
	 *
	 * @param filename Filename Name of the target file.
	 * @param buffer   buffer to write stats to
	 *
	 * @returns Upon successful completion, the file descriptor of the
	 * file is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern int nanvix_vfs_stat(const char *filename, struct nanvix_stat *restrict buffer);

	/**
	 * @brief Opens a file.
	 *
	 * @param filename Filename Name of the target file.
	 * @param oflag    Open flag.
	 *
	 * @returns Upon successful completion, the file descriptor of the
	 * opened file is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern int nanvix_vfs_open(const char *filename, int oflag, mode_t mode);

	/**
	 * @brief Closes a file.
	 *
	 * @param fd Target file descriptor.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_vfs_close(int fd);

	/**
	 * @brief Unlinks a file.
	 *
	 * @param filename Target filename
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_vfs_unlink(const char *filename);

	/**
	 * @brief Repositions the read/write pointer of a file.
	 *
	 * @param fd     Target file descriptor.
	 * @param offset Offset for read/write pointer.
	 * @param whence Reposition base location.
	 *
	 * @returns Upon successful completion, the resulting offset
	 * location is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern off_t nanvix_vfs_seek(int fd, off_t offset, int whence);

	/**
	 * @brief Reads data from a file.
	 *
	 * @param fd  Target file descriptor.
	 * @param buf Target buffer.
	 * @param n   Number of bytes to read.
	 *
	 * @returns Upon successful completion, the number of bytes
	 * successfully read is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	extern ssize_t nanvix_vfs_read(int fd, void *buf, size_t n);

	/**
	 * @brief Writes data to a file.
	 *
	 * @param fd  Target file descriptor.
	 * @param buf Target buffer.
	 * @param n   Number of bytes to write.
	 *
	 * @returns Upon successful completion, the number of bytes
	 * successfully written is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	extern ssize_t nanvix_vfs_write(int fd, const void *buf, size_t n);

#endif /* NANVIX_RUNTIME_FS_VFS_H_ */
