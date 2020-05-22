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

#ifndef NANVIX_SERVERS_VFS_FS_H_
#define NANVIX_SERVERS_VFS_FS_H_

	#ifndef __VFS_SERVER
	#error "do not include this file"
	#endif

	#include <posix/sys/types.h>
	#include "minix.h"

	/**
	 * @brief Length for Table of Files
	 */
	#define NANVIX_NR_FILES 64

	/**
	 * @brief In-Memory Inode
	 */
	struct inode
	{
		struct d_inode data; /**< Underlying Disk Inode  */
		dev_t dev;           /**< Underlying Device      */
		ino_t num;           /**< Inode Number           */
		int count;           /**< Reference count        */
	};

	/**
	 * @brief File
	 */
	struct file
	{
		int oflag;           /* Open flags.                   */
		int count;           /* Reference count.              */
		off_t pos;           /* Read/write cursor's position. */
		struct inode *inode; /* Underlying inode.             */
	};

	/**
	 * @brief Initializes the file system manager.
	 */
	extern void fs_init(void);

	/**
	 * @brief Gets an empty file.
	 *
	 * The getfile() function searches the table of files for a free entry.
	 *
	 * @returns Upon successful completion, a pointer to an empty file is
	 * returned. Upon failure, a NULL pointer is returned instead.
	 */
	extern struct file *getfile(void);

	/**
	 * @brief Opens a file.
	 *
	 * @param filename Filename Name of the target file.
	 * @param oflag    Open flag.
	 * @param mode     Access mode.
	 *
	 * @returns Upon successful completion, the file descriptor of the
	 * opened file is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern int fs_open(const char *filename, int oflag, mode_t mode);

	/**
	 * @brief Closes a file.
	 *
	 * @param fd Target file descriptor.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int fs_close(int fd);

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
	extern ssize_t fs_read(int fd, void *buf, size_t n);

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
	extern ssize_t fs_write(int fd, void *buf, size_t n);

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
	extern off_t fs_lseek(int fd, off_t offset, int whence);

	/**
	 * @brief Reads data from a regular file.
	 *
	 * @param ip  Inode of target regular file.
	 * @param buf Target buffer.
	 * @param n   Number of bytes to read.
	 * @param off Read offset.
	 *
	 * @returns Upon successful completion, the number of bytes
	 * successfully read is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	extern ssize_t file_read(
		struct inode *ip,
		void *buf,
		size_t n,
		off_t off
	);

	/**
	 * @brief Writes data to a regular file.
	 *
	 * @param ip  Inode of target regular file.
	 * @param buf Target buffer.
	 * @param n   Number of bytes to read.
	 * @param off Write offset.
	 *
	 * @returns Upon successful completion, the number of bytes
	 * successfully written is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	extern ssize_t file_write(
		struct inode *ip,
		void *buf,
		size_t n,
		off_t off
	);

	/**
	 * @brief Allocates an in-memory inode.
	 *
	 * @param mode Access permissions.
	 * @param uid  User ID of the onwer.
	 * @param gid  Group ID of the owner.
	 *
	 * @returns Upon successful completion, an in-memory pointer to the
	 * allocated inode is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern struct inode *inode_alloc(mode_t mode, uid_t uid, gid_t gid);

	/**
	 * @brief Releases an in-memory inode.
	 *
	 * @param ip Target inode.
	 *
	 * @returns Upon successful completion, zero is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	extern int inode_free(struct inode *ip);

	/**
	 * @brief Reads an inode to memory.
	 *
	 * @param num Number of the target inode.
	 *
	 * @returns Upon successful completion, a pointer to the target inode is
	 * returned. Upon failure, a negative error code is returned instead.
	 */
	extern struct inode *inode_read(ino_t num);

	/**
	 * @brief Writes an in-memory inode back to disk.
	 *
	 * @param num Number of the target inode.
	 *
	 * @returns Upon successful completion, zero is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	extern int inode_write(struct inode *ip);

#endif /* NANVIX_SERVERS_VFS_FS_H_*/
