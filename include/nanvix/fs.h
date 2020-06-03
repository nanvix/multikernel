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

#ifndef NANVIX_FS_H_
#define NANVIX_FS_H_

	#include <nanvix/runtime/fs.h>
	#include <nanvix/servers/vfs.h>
	#include <posix/unistd.h>

	/**
	 * @brief Asserts if file access mode is O_RDONLY.
	 *
	 * @param x Target mode
	 *
	 * @returns Non-zero if @p x equals to @p O_RDONLY and zero otherwise.
	 */
	#define ACCMODE_RDONLY(x) (ACCMODE(x) == O_RDONLY)

	/**
	 * @brief Asserts if file access mode is O_WRONLY.
	 *
	 * @param x Target mode
	 *
	 * @returns Non-zero if @p x equals to @p O_WRONLY and zero otherwise.
	 */
	#define ACCMODE_WRONLY(x) (ACCMODE(x) == O_WRONLY)

	/**
	 * @brief Asserts if file access mode is O_RDWR.
	 *
	 * @param x Target mode
	 *
	 * @returns Non-zero if @p x equals to @p O_RDWR and zero otherwise.
	 */
	#define ACCMODE_RDWR(x) (ACCMODE(x) == O_RDWR)

	/**
	 * @brief Asserts if a file descriptor is valid.
	 *
	 * @param x
	 *
	 * @returns Non-zero if @p x refers to a valid file descriptor and zero
	 * otherwise.
	 */
	#define NANVIX_VFS_FD_IS_VALID(x) \
		(WITHIN(fd, 0, NANVIX_OPEN_MAX))

#endif /* NANVIX_FS_H_ */
