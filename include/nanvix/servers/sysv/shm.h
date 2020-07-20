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

#ifndef NANVIX_SERVERS_SYSV_SHM_H_
#define NANVIX_SERVERS_SYSV_SHM_H_

#if defined(__NEED_SYSV_SERVER) || defined(__SYSV_SERVER)

	/* Must come first. */
	#define __NEED_LIMITS_PM
	#define __NEED_TYPES_PM

	#include <nanvix/limits/pm.h>
	#include <nanvix/types.h>
	#include <nanvix/ulib.h>
	#include <posix/sys/types.h>

	/**
	 * @name Types of Messages
	 */
	/**@{*/
	#define SYSV_SHM_OPEN      11 /**< Open     */
	#define SYSV_SHM_CREATE    12 /**< Create   */
	#define SYSV_SHM_UNLINK    13 /**< Unlink   */
	#define SYSV_SHM_CLOSE     14 /**< Close    */
	#define SYSV_SHM_FTRUNCATE 15 /**< Truncate */
	#define SYSV_SHM_INVAL     16 /**< Truncate */
	#define SYSV_SHM_SUCCESS   17 /**< Success  */
	#define SYSV_SHM_FAIL      18 /**< Failure  */
	/**@}*/

	/**
	 * @brief Payload for Semaphore Message
	 */
	union shm_payload
	{
		/* Create message. */
		struct {
			char name[NANVIX_SHM_NAME_MAX]; /**< Name of Shared Memory Region */
			int oflags;                     /**< Opening Flags                */
			mode_t mode;                    /**< Access Permission            */
		} create;

		/* Open message. */
		struct {
			char name[NANVIX_SHM_NAME_MAX]; /**< Name of Shared Memory Region */
			int oflags;                     /**< Opening Flags                */
		} open;

		/* Unlink message. */
		struct {
			char name[NANVIX_SHM_NAME_MAX]; /**< Shared Memory Region name. */
		} unlink;

		/* Close message. */
		struct {
			int shmid; /**< ID of shared memory region.  */
		} close;

		/**
		 * Truncate message.
		 */
		struct {
			int shmid;  /**< Target shared memory region. */
			off_t size; /**< Size (in bytes).             */
		} ftruncate;

		/**
		 * Invalidate message.
		 */
		struct {
			int shmid;    /**< Target Shared Memory Region */
			rpage_t page; /**< Target Page                 */
		} inval;
	};

	/**
	 * @brief Asserts whether or not a shared memory region ID is valid.
	 *
	 * @param shmid ID of the target shared memory region.
	 *
	 * @returns Non-zero if the shared memory region valid, and zero otherwise.
	 */
	static inline int nanvix_shm_is_valid(int shmid)
	{
		return ((shmid >= 0) && (shmid < NANVIX_SHM_MAX));
	}

	/**
	 * @brief Asserts if a shared memory region has a valid name.
	 *
	 * @param name Target name.
	 *
	 * @returns Zero if the target @p name is valid and a negative error
	 * code otherwise.
	 */
	static inline int nanvix_shm_name_is_invalid(const char *name)
	{
		/* Invalid name. */
		if ((name == NULL) || (!ustrcmp(name, "")))
			return (-EINVAL);

		/* Name too long. */
		if (ustrlen(name) >= (NANVIX_SHM_NAME_MAX - 1))
			return (-ENAMETOOLONG);

		return (0);
	}

#ifdef __SYSV_SERVER

	/**
	 * @brief Creates a shared memory region
	 *
	 * @param page   Underlying page of shared memory region.
	 * @param proc   ID of owner process.
	 * @param name   Name of the targeted shm.
	 * @param oflags Opening flags.
	 * @param mode   Access permissions.
	 *
	 * @returns Upon successful completion, the ID of the newly created
	 * opened shared memory region is returned. Upon failure, a negative
	 * error code is returned instead.
	 */
	extern int __do_shm_create(
			rpage_t *page,
			nanvix_pid_t proc,
			const char *name,
			int oflags,
			mode_t mode
	);

	/**
	 * @brief Opens a shared memory region
	 *
	 * @param page   Underlying page of shared memory region.
	 * @param proc   ID of opening process.
	 * @param name   Name of the targeted shared memory region.
	 * @param oflags Opening flags.
	 *
	 * @returns Upon successful completion, the shared memory region ID is
	 * returned. Upon failure, a negative error code is returned instead.
	 */
	extern int __do_shm_open(
		rpage_t *page,
		nanvix_pid_t proc,
		const char *name,
		int oflags
	);

	/**
	 * @brief Close a opened shared memory region
	 *
	 * @param proc  ID of opening process.
	 * @param shmid ID of the target shared memory region.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __do_shm_close(nanvix_pid_t proc, int shmid);

	/**
	 * @brief Unlink a shared memory region
	 *
	 * @param proc ID of opening process.
	 * @param name Name of the targeted shm.
	 *
	 * @returns Upon successful completion, oshmid is returned.
	 * Upon failure, a negative error code is returned instead.
	 */
	extern int __do_shm_unlink(nanvix_pid_t proc, const char *name);

	/**
	 * @brief Truncates a shared memory region to a size.
	 *
	 * @param page  Underlying page of shared memory region.
	 * @param proc  ID of opening process.
	 * @param shmid ID of the target shared memory region.
	 * @param size  New size for the target shared memory region.
	 *
	 * @returns Upon sucessful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __do_shm_ftruncate(
		rpage_t *page,
		nanvix_pid_t proc,
		int shmid,
		off_t size
	);

	/**
	 * @brief Initializes the table of shared memory regions.
	 */
	extern void shm_init(void);

#endif /* __SYSV_SERVER */

#endif /* __NEED_SYSV_SERVER || __SYSV_SERVER */

#endif /* NANVIX_SERVERS_SYSV_SHM_H_ */

