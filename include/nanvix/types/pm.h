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

#ifndef NANVIX_TYPES_PM_H_
#define NANVIX_TYPES_PM_H_

	#ifndef __NEED_TYPES_PM
	#error "do not include this file"
	#endif

	#include <posix/sys/types.h>
	#include <posix/sys/ipc.h>

	/**
	 * @brief IPC Key
	 */
	typedef key_t nanvix_key_t;

	/**
	 * @brief File Attributes
	 */
	typedef mode_t nanvix_mode_t;

	/**
	 * @brief Process's ID
	 */
	typedef pid_t nanvix_pid_t;

	/**
	 * @brief User's ID
	 */
	typedef uid_t nanvix_uid_t;

	/**
	 * @brief Group's ID
	 */
	typedef gid_t nanvix_gid_t;

	/**
	 * @brief Creator's ID
	 */
	typedef cid_t nanvix_cid_t;

	/**
	 * @brief Creator's Group ID
	 */
	typedef cgid_t nanvix_cgid_t;

	/**
	 * @brief Semaphore Buffer
	 */
	struct nanvix_sembuf
	{
		unsigned short sem_num; /**< Semaphore Number    */
		short sem_op;           /**< Semaphore Operation */
		short sem_flg;          /**< Operation Flags     */
	};

	/**
	 * @brief IPC Permissions
	 */
	struct nanvix_ipc_perm
	{
		nanvix_uid_t uid;   /**< Owner's User ID       */
		nanvix_gid_t gid;   /**< Owner's Group ID      */
		nanvix_uid_t cuid;  /**< Creator's User ID     */
		nanvix_gid_t cgid;  /**< Creator's Group ID    */
		nanvix_mode_t mode; /**< Read/Write Permission */
	};

#endif /* NANVIX_TYPES_PM_H_ */
