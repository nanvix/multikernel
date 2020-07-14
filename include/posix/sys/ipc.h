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

#ifndef POSIX_SYS_IPC_H_
#define POSIX_SYS_IPC_H_

	/**
	 * @group IPC Mode Bits
	 */
	/**@{*/
	#define IPC_CREAT   (1 << 0) /**< Create entry if key does not exist. */
	#define IPC_EXCL    (1 << 1) /**< Fail if key exists.                 */
	#define IPC_NOWAIT  (1 << 2) /**< Error if request must wait.         */
	/**@}*/

	/**
	 * @brief IPC Key
	 */
	/**@{*/
	#define IPC_PRIVATE (1 << 3) /**< Private Key */
	/**@}*/

	/**
	 * @brief IPC Control Commands
	 */
	/**@{*/
	#define IPC_RMID (1 << 4) /**< Remove Identifier */
	#define IPC_SET  (1 << 5) /**< Set Options       */
	#define IPC_STAT (1 << 6) /**< Get Options       */
	/**@}*/

	/**
	 * @brief Creator's ID
	 */
	typedef int cid_t;

	/**
	 * @brief Creator's Group ID
	 */
	typedef int cgid_t;

#endif /* POSIX_SYS_IPC_H_ */
