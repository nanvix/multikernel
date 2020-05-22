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

#ifndef POSIX_FCNTL_H_
#define POSIX_FCNTL_H_

	/**
	 * @name Access Modes
	 */
	/**@{*/
	#define O_ACCMODE 00003 /**< Mask            */
	#define O_RDONLY     00 /**< Read Only       */
	#define O_WRONLY     01 /**< Write Only      */
	#define O_RDWR       02 /**< Read and wWrite */
	/**@}*/

	/**
	 * @name File Creation Flags
	 */
	/**@{*/
	#define O_CREAT  00100 /**< Create File                       */
	#define O_EXCL	 00200 /**< Exclusive Use Flag                */
	#define O_NOCTTY 00400 /**< Don't Assign Controlling Terminal */
	#define O_TRUNC	 01000 /**< Truncate File                     */
	/**@}*/

	/**
	 * @name File Status Flags
	 */
	/**@{*/
	#define O_APPEND   02000 /**< Append mode.       */
	#define O_NONBLOCK 04000 /**< Non-blocking mode. */
	/**@}*/

	/**
	 * @brief Returns access mode of a file.
	 */
	#define ACCMODE(m) (m & O_ACCMODE)

#endif /* POSIX_FCNTL_H_ */
