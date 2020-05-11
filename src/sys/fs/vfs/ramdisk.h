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

#ifndef RAMDISK_H_
#define RAMDISK_H_

	#include <posix/sys/types.h>

	/**
	 * @brief Writes data to a RAM Disk.
	 *
	 * @param minor Target RAM Disk.
	 * @param buf   Target buffer from where data should be read.
	 * @param n     Number of bytes to read.
	 * @param off   Read offset.
	 *
	 * @returns Upon successful completion, the number of bytes
	 * effectively read is returned. Upon failure, a negative error code
	 * is returned instead.
	 */
	extern ssize_t ramdisk_write(unsigned minor, const char *buf, size_t n, off_t off);

	/**
	 * @brief Reads data from a RAM disk.
	 *
	 * @param minor Target RAM Disk.
	 * @param buf   Target buffer to where data should be written.
	 * @param n     Number of bytes to write.
	 * @param off   Write offset.
	 *
	 * @returns Upon successful completion, the number of bytes
	 * effectively wrriten is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	extern ssize_t ramdisk_read(unsigned minor, char *buf, size_t n, off_t off);

	/**
	 * @brief Initializes the RAM Disk devices.
	 */
	extern void ramdisk_init(void);

#endif /* RAMDISK_H_ */
