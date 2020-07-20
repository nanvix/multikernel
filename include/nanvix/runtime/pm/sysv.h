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

#ifndef NANVIX_RUNTIME_PM_SYSV_H_
#define NANVIX_RUNTIME_PM_SYSV_H_

	#define __NEED_TYPES_PM

	#include <nanvix/types/pm.h>
	#include <posix/sys/types.h>
	#include <posix/fcntl.h>

#ifdef __SYSV_SERVICE

	#include <nanvix/servers/sysv.h>

	/**
	 * @brief Assets wheter or not the client is initialzied
	 *
	 * @returns Non-zero if the client is initialized and zero
	 * otherwise.
	 */
	extern int __nanvix_sysv_is_initialized(void);

	/**
	 * @brief Gets the outbox to the server.
	 *
	 * @returns The outbox to the server.
	 */
	extern int __nanvix_sysv_outbox(void);

	/**
	 * @brief Gets the outportal to the server.
	 *
	 * @returns The outportal to the server.
	 */
	extern int __nanvix_sysv_outportal(void);

#endif /* __SYSV_SERVICE */

	/**
	 * @brief Initializes the System V Service client.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_sysv_setup(void);

	/**
	 * @brief Shutdowns the System V Service client.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_sysv_cleanup(void);

	/**
	 * @brief Shutdowns the sysv server.
	 *
	 * @returns Upon successful completion 0 is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_sysv_shutdown(void);

/*============================================================================*
 * Shared Memory Regions                                                      *
 *============================================================================*/

	/**
	 * @brief Initializes the SHM Service.
	 *
	 * @returns Upon successful completion, zero is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	extern int __nanvix_shm_setup(void);

	/**
	 * @brief Shutdowns the SHM Service.
	 */
	extern int __nanvix_shm_cleanup(void);

	/**
	 * @brief Opens a shared memory region.
	 *
	 * @param name   Name of the new shared memory region.
	 * @param oflags Opening flags.
	 * @param mode   Access permissions.
	 *
	 * @returns Upon successful completion, a descriptor for the target
	 * shared memory region is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	extern int __nanvix_shm_open(const char *name, int oflags, mode_t mode);

	/**
	 * @brief Creates a shared memory region.
	 *
	 * @param name Name of the new shared memory region.
	 * @param mode Access permissions.
	 *
	 * @returns Upon successful completion, a descriptor for the target
	 * shared memory region is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	static inline int __nanvix_shm_creat(const char *name, mode_t mode)
	{
		return (__nanvix_shm_open(name, O_WRONLY | O_CREAT | O_TRUNC, mode));
	}

	/**
	 * @brief Removes a shared memory region.
	 *
	 * @param name Name of the target existing shared memory region.
	 *
	 * @returns Upon successful completion, a descriptor for the target
	 * shared memory region is returned. Upon failure, a negative error
	 * code is returned instead.
	 */
	extern int __nanvix_shm_unlink(const char *name);

	/**
	 * @brief Closes a shared memory region.
	 *
	 * @param shmid ID of the target shared memory region.
	 *
	 * @returns Upon sucessful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_shm_close(int shmid);

	/**
	 * @brief Truncates a shared memory region to a size.
	 *
	 * @param shmid ID of the target shared memory region.
	 * @param size  New size for the target shared memory region.
	 *
	 * @returns Upon sucessful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_shm_ftruncate(int shmid, off_t size);

	/**
	 * @brief Reads data from a shared memory region.
	 *
	 * @param shmid ID of the target shared memory region.
	 * @param buf   Location where the data should be written to.
	 * @param n     Number of bytes to read.
	 * @param off   Read offset within the shared memory region.
	 *
	 * @returns Upon successful completion, the number of bytes actualy
	 * read is returned. Upon failure, a negative error code is returned
	 * instead.
	 */
	extern ssize_t __nanvix_shm_read(int shmid, void *buf, size_t n, off_t off);

	/**
	 * @brief Writes data to a shared memory region.
	 *
	 * @param shmid ID of the target shared memory region.
	 * @param buf   Location where the data should be read from.
	 * @param n     Number of bytes to write.
	 * @param off   Read offset within the shared memory region.
	 *
	 * @returns Upon successful completion, the number of bytes actualy
	 * written is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern ssize_t __nanvix_shm_write(int shmid, const void *buf, size_t n, off_t off);

	/**
	 * @brief Broadcasts an invalidation signal on a shared memory region.
	 *
	 * @param shmid ID of the target shared memory region.
	 *
	 * @returns Upon sucessful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_shm_inval(int shmid);

/*============================================================================*
 * Message Queues                                                             *
 *============================================================================*/

	/**
	 * @brief Gets a message queue.
	 *
	 * @param key    Key for message queue.
	 * @param msgflg Flags.
	 *
	 * @returns Upon successful completion, the ID of the target message
	 * queue is returned. Upon failure, a negative error code is
	 * returned instead.
	 */
	extern int __nanvix_msg_get(key_t key, int msgflg);

	/**
	 * @brief Closes a message queue.
	 *
	 * @param msgid ID of the target message queue.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_msg_close(int msgid);

	/**
	 * @brief Puts a message in a message queue.
	 *
	 * @param msgid  ID of the target message queue.
	 * @param msgp   Pointer to the targe message.
	 * @param msgsz  Size of the message.
	 * @param msgflg Flags.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_msg_send(
		int msgid,
		const void *msgp,
		size_t msgsz,
		int msgflg
	);

	/**
	 * @brief Retrives a message from a message queue.
	 *
	 * @param msgid  ID of the target message queue.
	 * @param msgp   Pointer to the targe message.
	 * @param msgsz  Size of the message.
	 * @param msgtyp Type of the message.
	 * @param msgflg Flags.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_msg_receive(
		int msgid,
		void *msgp,
		size_t msgsz,
		long msgtyp,
		int msgflg
	);

/*============================================================================*
 * Semaphores                                                                 *
 *============================================================================*/

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
	extern int __nanvix_semget(key_t key, int semflg);

	/**
	 * @brief Closes a semaphore.
	 *
	 * @param semid ID of the target semaphore.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_sem_close(int semid);

	/**
	 * @brief Operates on a semaphore.
	 *
	 * @param semid ID of the target semaphore.
	 * @param sops  Semaphore operations.
	 * @param nsops Number of semaphore operations.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_semop(
		int semid,
		const struct nanvix_sembuf *sops,
		size_t nsops
	);

#endif /* NANVIX_RUNTIME_PM_SYSV_H_ */
