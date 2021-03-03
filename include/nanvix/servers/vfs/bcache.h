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

#ifndef NANVIX_SERVERS_VFS_BCACHE_H_
#define NANVIX_SERVERS_VFS_BCACHE_H_

	#ifndef __VFS_SERVER
	#error "do not include this file"
	#endif

	#include <posix/sys/types.h>
	#include "types.h"

 	/**
 	 * @addtogroup Buffer
 	 */
	/**@{*/

	/**
	 * @brief Opaque Buffer
	 */
	struct buffer;

	/**
	 * @brief Initializes the bock cache.
	 */
	extern void binit(void);

	/**
	 * @brief Returns the size in bytes of the buffer struct
	 */
	extern int buffer_get_size(void);

	/**
	 * @brief Gets a refecente to the underlying data of a block buffer.
	 *
	 * @param buf Target block buffer.
	 *
	 * @returns Upon successful completion, a pointer to the underlying
	 * data of the target block buffer is returned. Upon failure, a NULL
	 * pointer is returned instead.
	 */
	extern void *buffer_get_data(struct buffer *buf);

	/**
	 * @brief Sets a block buffer as dirty
	 *
	 * @param buf Target block buffer.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int buffer_set_dirty(struct buffer *buf);

	/**
	 * @brief Asserts if a block buffer is dirty
	 *
	 * @param buf Target block buffer.
	 *
	 * @returns Zero if the target block buffer is not dirty and
	 * non-zero otherwise.
	 */
	extern int buffer_is_dirty(struct buffer *buf);

	/**
	 * @brief Reads a block from a device.
	 *
	 * @param dev Device number.
	 * @param num Block number.
	 *
	 * @returns Upon successful completion, a pointer to a buffer
	 * holding the requested block is returned. In this case, the block
	 * buffer is ensured to be locked. Upon failure, a NULL pointer is
	 * returned instead.
	 */
	extern struct buffer *bread(dev_t dev, block_t num);

	/**
	 * @brief Writes a block buffer to the underlying device.
	 *
	 * @param buf Target block buffer.
	 *
	 * @returns Upon successful completion zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int bwrite2(struct buffer *buf);

	/**
	 * @brief Writes a block buffer to the underlying device.
	 *
	 * @param buf Target block buffer.
	 *
	 * @returns Upon successful completion zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int bwrite(struct buffer *buf);

	/**
	 * @brief Releases a block.
	 *
	 * @param buf Target block.
	 *
	 * @returns Upon successful completion zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int brelse(struct buffer *buf);

	/**@}*/

#endif /* NANVIX_SERVERS_VFS_BCACHE_H_ */

