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

#ifndef NANVIX_RUNTIME_MM_MANAGER_H_
#define NANVIX_RUNTIME_MM_MANAGER_H_

#ifdef __NEED_MM_VMEM_MANAGER

	#include <nanvix/types/mm/rmem.h>
	#include <posix/stddef.h>

	/**
	 * @brief Handles a remote page fault.
	 *
	 * @param vaddr Faulting virtual address.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_rfault(vaddr_t vaddr);

	/**
	 * @brief Allocates remote memory.
	 *
	 * @param n Number of bytes to allocate.
	 *
	 * @returns Upon successful completion, a pointer to the newly
	 * allocated remote memory area is returned. Upon failure, a null
	 * pointer is returned instead.
	 */
	extern void *nanvix_vmem_alloc(size_t n);

	/**
	 * @brief Frees remote memory.
	 *
	 * @param ptr Target remote memory area.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_vmem_free(void *ptr);

	/**
	 * @brief Reads data from remote memory.
	 *
	 * @param buf Local buffer where data should be placed.
	 * @param ptr Target remote memory area.
	 * @param n   Number of bytes to read.
	 *
	 * @returns The number of bytes read from remote memory.
	 */
	extern size_t nanvix_vmem_read(void *buf, const void *ptr, size_t n);

	/**
	 * @brief Writes data to remote memory.
	 *
	 * @param ptr Target remote memory area.
	 * @param buf Local buffer from where data should be retrieved.
	 * @param n   Number of bytes to write.
	 *
	 * @returns The number of bytes written to remote memory.
	 */
	extern size_t nanvix_vmem_write(void *ptr, const void *buf, size_t n);

#endif /* __NEED_MAMAGER_SERVICE */

#endif /* NANVIX_RUNTIME_MM_MANAGER_H_ */

