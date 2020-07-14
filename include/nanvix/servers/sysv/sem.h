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

#ifndef NANVIX_SERVERS_SYSV_SEM_H_
#define NANVIX_SERVERS_SYSV_SEM_H_

#if defined(__NEED_SYSV_SERVER) || defined(__SYSV_SERVER)

	/* Must come first. */
	#define __NEED_LIMITS_PM

	#include <nanvix/limits/pm.h>
	#include <nanvix/types/pm.h>
	#include <posix/sys/types.h>

	/**
	 * @name Types of Messages
	 */
	/**@{*/
	#define SYSV_SEM_GET      (1 << 5) /**< Get Semaphore     */
	#define SYSV_SEM_CLOSE    (2 << 5) /**< Close Semaphore   */
	#define SYSV_SEM_OPERATE  (3 << 5) /**< Operate Semaphore */
	/**@}*/

	/**
	 * @brief Payload for Semaphore Message
	 */
	union sem_payload
	{
		/**
		 * @brief Get Semaphore
		 */
		struct
		{
			key_t key;  /**< Key  */
			int semflg; /**< Flag */
		} get;

		/**
		 * @brief Close Semaphore
		 */
		struct
		{
			int semid; /**< ID */
		} close;

		/**
		 * @brief Operate Semaphore
		 */
		struct
		{
			int semid;                   /**< ID               */
			struct nanvix_sembuf sembuf; /**< Operation Buffer */
		} operate;
	};

	/**
	 * @brief Asserts if a semaphore is valid.
	 *
	 * @param x Target semaphore.
	 *
	 * @returns Non zero if the target semaphore is valid and zero
	 * otherwise.
	 */
	#define semid_is_valid(x) (((x) >= 0) && ((x) < NANVIX_SEM_MAX))

#ifdef __SYSV_SERVER

	/**
	 * @brief Gets a semaphore.
	 *
	 * @param key    Key for semaphore.
	 * @param semflg Flags.
	 *
	 * @returns Upon successful completion, the ID of the target
	 * semaphore is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern int do_sem_get(key_t key, int semflg);

	/**
	 * @brief Closes a semaphore.
	 *
	 * @param semid ID of the target semaphore.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int do_sem_close(int semid);

	/**
	 * @brief Operates on a semaphore.
	 *
	 * @param pid   ID of the calling process.
	 * @param semid ID of the target semaphore.
	 * @param sops  Semaphore operations.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern nanvix_pid_t do_sem_operate(
		nanvix_pid_t pid,
		int semid,
		const struct nanvix_sembuf *sops
	);

	/**
	 * @brief Initializes the semaphores service.
	 */
	extern void do_sem_init(void);

#endif /* __SYSV_SERVER */

#endif /* __NEED_SYSV_SERVER || __SYSV_SERVER */

#endif /* NANVIX_SERVERS_SYSV_SEM_H_ */
