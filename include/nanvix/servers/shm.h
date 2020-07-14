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

#ifndef NANVIX_SERVERS_SHM_H_
#define NANVIX_SERVERS_SHM_H_

#if defined(__NEED_MM_SHM_SERVER) || defined(__SHM_SERVER)

	/* Must come first. */
	#define __NEED_LIMITS_SHM

	#include <nanvix/servers/message.h>
	#include <nanvix/limits/shm.h>
	#include <nanvix/types/mm/rmem.h>
	#include <posix/sys/types.h>
	#include <nanvix/types.h>
	#include <nanvix/ulib.h>
	#include <posix/errno.h>
	#include <posix/stddef.h>

	/**
	 * @bried Shared memory region operations.
	 */
	/**@{*/
	#define SHM_EXIT         0  /**< Exit Request.     */
	#define SHM_OPEN         1  /**< Open.             */
	#define SHM_CREATE       2  /**< Create.           */
	#define SHM_UNLINK       3  /**< Unlink.           */
	#define SHM_CLOSE        4  /**< Close             */
	#define SHM_FTRUNCATE    5  /**< Truncate.         */
	#define SHM_INVAL        6  /**< Truncate.         */
	#define SHM_SUCCESS      7  /**< Success.          */
	#define SHM_FAIL         8  /**< Failure.          */
	/**@}*/

	/**
	 * @brief Shared Memory Region message.
	 */
	struct shm_message
	{
		message_header header; /**< Message header.  */

		/* Operation-specific fields. */
		union
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

			/* Return message. */
			struct
			{
				int shmid;    /**< ID of shared memory region.  */
				int status;   /**< Status code.                 */
				rpage_t page; /**< Base Address                 */
			} ret;
		} op;
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

#ifdef __SHM_SERVER

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

	/**
	 * @brief Debug SHM Server?
	 */
	#define __DEBUG_SHM 0

	#if (__DEBUG_SHM)
	#define shm_debug(fmt, ...) uprintf(fmt, __VA_ARGS__)
	#else
	#define shm_debug(fmt, ...) { }
	#endif

#endif /*__SHM_SERVER */

#endif /* defined(__NEED_SHM_SERVER) || defined(__SHM_SERVER) */

#endif /* _MAILBOX_H_ */
