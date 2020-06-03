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

#ifndef NANVIX_RUNTIME_MM_STUB_H_
#define NANVIX_RUNTIME_MM_STUB_H_

#ifdef __NEED_MM_RMEM_STUB

	#include <nanvix/types/mm/rmem.h>

	#define __NEED_RMEM_SERVER

	#include <nanvix/servers/rmem.h>

	/**
	 * @brief Allocates a remote memory block.
	 *
	 * @returns Upon successful completion, the number of the newly
	 * allocated block in the remote memory is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	extern rpage_t nanvix_rmem_alloc(void);

	/**
	 * @brief Frees a remote memory block.
	 *
	 * @param blknum Number of the target block.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rmem_free(rpage_t blknum);

	/**
	 * @brief Reads data from the remote memory.
	 *
	 * @param blknum Number of the target block.
	 * @param buf    Location where the data should be written to.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern size_t nanvix_rmem_read(rpage_t blknum, void *buf);

	/**
	 * @brief Writes data to the remote memory.
	 *
	 * @param blknum Number of the target block.
	 * @param buf    Location where the data should be read from.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern size_t nanvix_rmem_write(rpage_t blknum, const void *buf);

	/**
	 * @brief Shutdowns aall remote memory servers.
	 *
	 * @returns Upon successful completion 0 is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rmem_shutdown(void);

#endif /* __NEED_MM_RMEM_STUB */

#endif /* NANVIX_RUNTIME_MM_STUB_H_ */

