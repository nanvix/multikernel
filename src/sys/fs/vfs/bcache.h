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

#ifndef _BCACHE_H_
#define _BCACHE_H_

	/* Must come first. */
	#define __NEED_RESOURCE

	#include <nanvix/hal/resource.h>
	#include <posix/sys/types.h>
	#include <posix/stdint.h>

 	/**
 	 * @addtogroup Buffer
 	 */
	/**@{*/

	/**
	 * @brief Block Size (in bytes)
	 */
	#define NANVIX_FS_BLOCK_SIZE 512

	/**
	 * @brief Block Number
	 */
	typedef uint32_t block_t;

	/**
	 * @brief Block Buffer
	 */
	struct buffer
	{
		/**
		 * @name Status information
		 */
		/**@{*/
		/* Must come first. */
		struct resource flags; /**< Flags */
		/**@}*/

		/**
		 * @name General information
		 */
		/**@{*/
		dev_t dev;                        /**< Device.          */
		block_t num;                      /**< Block number.    */
		char data[NANVIX_FS_BLOCK_SIZE];  /**< Underlying data. */
		int count;                        /**< Reference count. */
		/**@}*/
	};

	/**@}*/

	/**
	 * @brief Initializes the bock cache.
	 */
	extern void binit(void);

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

#endif /* _BCACHE_H_ */

