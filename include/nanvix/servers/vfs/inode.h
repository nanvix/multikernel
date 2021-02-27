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
#ifndef NANVIX_SERVERS_VFS_INODE_H_
#define NANVIX_SERVERS_VFS_INODE_H_

	#include "fs.h"

	/**
	 * @brief In-Memory Inode
	 */
	struct inode;

	/**
	 * The inode_disk_get() function gets the reference for the underlying
	 * disk inode.
	 */
	struct d_inode *inode_disk_get(struct inode *ip);

	/**
 	* The inode_get_count() function gets the inode count of the inode pointed to
 	* by @p ip.
 	*/
	int inode_get_count(const struct inode *ip);

	/**
	* The inode_set_count() function sets the inode count of the inode pointed to
	* by @p ip to the value of @p c.
	*/
	int inode_set_count(struct inode *ip, const int c);

	/**
 	 * The inode_increase_count() function increases the inode count of the inode pointed to
 	 * by @p ip by 1
 	 */
	int inode_increase_count(struct inode *ip);

	/**
 	 * The inode_decrease_count() function decreases the inode count of the  inode pointed to
 	 * by @p ip by 1
 	 */
	int inode_decrease_count(struct inode *ip);

	/**
	 * The inode_get_num() function gets the number of the inode pointed to
	 * by @p ip.
	 */
	ino_t inode_get_num(const struct inode *ip);

	/**
	 * The inode_null() function zeroes the number of the inode pointed to
	 * by @p ip.
	 */
	void inode_null(const struct inode *ip);

	/**
	 * The inode_get_dev() function gets the device number of the inode
	 * pointed to by @p ip.
	 */
	dev_t inode_get_dev(const struct inode *ip);

	/**
	 * The inode_set_dirty() sets the inode pointed to by @p ip as dirty.
	 */
	int inode_set_dirty(struct inode *ip);

	/**
	 * @brief Reads an inode to memory.
	 *
	 * @param fs   Target file system.
	 * @param num Number of the target inode.
	 *
	 * @returns Upon successful completion, a pointer to the target inode is
	 * returned. Upon failure, a negative error code is returned instead.
	 */
	static struct inode *inode_read(struct filesystem *fs, ino_t num);

	/**
	 * The inode_touch() function updates the time stamp of the inode
	 * pointed to by @p ip.
	 */
	int inode_touch(struct inode *ip);

	/**
	 * @brief Releases an in-memory inode.
	 *
	 * @param fs   Target file system.
	 * @param ip Target inode.
	 *
	 * @returns Upon successful completion, zero is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	static int inode_free(struct filesystem *fs, struct inode *ip);

	/**
	 * @brief Releases the reference to an inode.
	 *
	 * @param fs Target file system.
	 * @param ip Target inode.
	 *
	 * @returns Upon successful completion, zero is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	int inode_put(struct filesystem *fs, struct inode *ip);

	/**
	 * @todo TODO: Provide a detailed description for this function.
	 */
	int inode_write(struct filesystem *fs, struct inode *ip);

	/**
	 * The inode_get() function gets a reference to the inode specified by
	 * @p num that resides in the file system pointed to by @p fs.
	 */
	struct inode *inode_get(struct filesystem *fs, ino_t num);

	/**
	 * @todo TODO: Provide a detailed description for this function.
	 */
	struct inode *inode_alloc(
		struct filesystem *fs,
		mode_t mode,
		uid_t uid,
		gid_t gid
	);

	/**
	 * The inode_name() function lookups an inode by the name pointed to by
	 * @p name. The root directory of @p fs is used as start point for the
	 * search.
	 *
	 * @todo TODO: recursive lookup.
	 * @todo TODO: support relative path search.
	 */
	struct inode *inode_name(struct filesystem *fs, const char *name);

	/**
	 * The inode_init() function initializes the inodes module.
	 */
	void inode_init(void);

#endif
