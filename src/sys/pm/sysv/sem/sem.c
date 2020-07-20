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

/* Must come first.*/
#define __NEED_RESOURCE
#define __SYSV_SERVER

#include <nanvix/servers/sysv.h>
#include <nanvix/types.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

/**
 * @brief  Sleeping processes.
 */
static struct
{
	nanvix_pid_t pid; /**< ID of Sleeping Process */
	int semid;        /**< ID of Target Semaphore */
	int val;          /**< Target Value.          */
} sleeping[NANVIX_PROC_MAX];

/**
 * @brief Table of semaphores.
 */
static struct sem
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource; /**< Generic resource information.  */

	nanvix_pid_t owner;       /**< ID of owner process.  */
	key_t key;                /**< Key.                  */
	int refcount;             /**< Number of references. */
	mode_t mode;              /**< Access permissions.   */
	int val;                  /**< Counter               */
} semaphores[NANVIX_SEM_MAX];

/**
 * @brief Pool of semaphores.
 */
static struct resource_pool pool = {
	semaphores, NANVIX_SEM_MAX , sizeof(struct sem)
};

/*============================================================================*
 * do_sem_get()                                                               *
 *============================================================================*/

/**
 * The do_sem_get() function retrieves the semaphore that matches the @p
 * key parameter. The @p semflg flags specifies actions to take during
 * this operation. Upon successful completo, the ID of the target
 * semaphore is returned.
 *
 * @author Pedro Henrique Penna
 */
int do_sem_get(key_t key, int semflg)
{
	int semid;

	sysv_debug("do_sem_get() key=%d, nsems=%d, semflg=%x", key, semflg);

	/* Not supported. */
	if (key == IPC_PRIVATE)
		return (-ENOTSUP);

	/* Invalid flags. */
	if (!(semflg & IPC_CREAT) && (semflg & IPC_EXCL))
		return (-EINVAL);

	/* Search for key. */
	for (int i = 0; i < NANVIX_SEM_MAX; i++)
	{
		/* Skip invalid entries. */
		if (!resource_is_used(&semaphores[i].resource))
			continue;

		/* Found. */
		if (semaphores[i].key == key)
		{
			/* Semaphore exists. */
			if ((semflg & IPC_CREAT) && (semflg & IPC_EXCL))
				return (-EEXIST);

			semid = i;
			goto found;
		}
	}

	/* Do not create semaphore. */
	if (!(semflg & IPC_CREAT))
		return (-ENOENT);

	/* Allocate semaphore. */
	if ((semid = resource_alloc(&pool)) < 0)
		return (-ENOSPC);

	semaphores[semid].key = key;
	semaphores[semid].val = 0;

found:

	semaphores[semid].refcount++;

	return (semid);
}

/*============================================================================*
 * do_sem_close()                                                             *
 *============================================================================*/

/**
 * The do_sem_close() function closes the semaphore identified by @p
 * semid. Upon successful completo, zero is returned.
 *
 * @author Pedro Henrique Penna
 */
int do_sem_close(int semid)
{
	sysv_debug("do_sem_close() semid=%d", semid);

	/* Invalid ID for semaphore. */
	if (!semid_is_valid(semid))
		return (-EINVAL);

	/* Bad ID for semaphore. */
	if (!resource_is_used(&semaphores[semid].resource))
		return (-EINVAL);

	semaphores[semid].refcount--;

	/* Release semaphore. */
	if (semaphores[semid].refcount == 0)
		resource_free(&pool, semid);

	return (0);
}

/*============================================================================*
 * do_sem_operate()                                                           *
 *============================================================================*/

/**
 * @brief Puts a process to sleep.
 *
 * @param pid   ID of the target process.
 * @param semid ID of the target semaphore.
 * @param val   Sleeping val.
 *
 * @author Pedro Henrique Penna
 */
static void do_sleep(nanvix_pid_t pid, int semid, int val)
{
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		if (sleeping[i].semid == -1)
		{
			sleeping[i].pid = pid;
			sleeping[i].semid = semid;
			sleeping[i].val = val;
		}
	}
}

/**
 * @brief Wakes up a process.
 *
 * @param semid ID of the target semaphore.
 *
 * @returns If a process that can be awaken is found, the ID of such
 * process is returned. Otherwise, zero is returned instead.
 *
 * @author Pedro Henrique Penna
 */
static nanvix_pid_t do_wakeup(int semid)
{
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		/* Skip invalid processes. */
		if (sleeping[i].semid != semid)
			continue;

		/* Found. */
		if (sleeping[i].val <= semaphores[semid].val)
		{
			sleeping[i].semid = -1;
			semaphores[semid].val -= sleeping[i].val;

			return (sleeping[i].pid);
		}
	}

	return (0);
}

/**
 * The do_sem_operate() function operates on the semaphore identied by
 * @p semid. The @p sops parameter speficies the operation to execute.
 * Upon successful completion, this function returns either: (i) zero
 * (indicating tha the operation was promptly completed and no futher
 * action is required; or (ii) a positive process identifier that
 * signals, which in turn provides additional information on whether the
 * calling process should block (return value equals @p pid parameter)
 * or another process should be awaken return value differs from @p pid
 * parameter).
 *
 * @author Pedro Henrique Penna
 */
nanvix_pid_t do_sem_operate(
	nanvix_pid_t pid,
	int semid,
	const struct nanvix_sembuf *sops
)
{
	sysv_debug("do_sem_operate() pid=%d, semid=%d, sops.val=%d",
		pid, semid, sops->sem_op
	);

	/* Invalid process. */
	if (pid < 0)
		return (-EINVAL);

	/* Invalid semaphore. */
	if (!semid_is_valid(semid))
		return (-EINVAL);

	/* Bad ID for semaphore. */
	if (!resource_is_used(&semaphores[semid].resource))
		return (-EINVAL);

	/* Invalid semaphore buffer. */
	if (sops == NULL)
		return (-EINVAL);

	/* Not supported. */
	if (sops->sem_flg & SEM_UNDO)
		return (-ENOTSUP);

	/* Increment semaphore. */
	if (sops->sem_op > 0)
	{
		nanvix_pid_t awaken;

		semaphores[semid].val += sops->sem_op;
		if ((awaken = do_wakeup(semid)) > 0)
			return awaken;
	}

	/* Decrement semaphore. */
	else if (sops->sem_op < 0)
	{
		if ((semaphores[semid].val + sops->sem_op) < 0)
		{
			/* Do not block. */
			if (sops->sem_flg & IPC_NOWAIT)
				return (0);

			do_sleep(pid, semid, sops->sem_op);
			return (pid);
		}

		semaphores[semid].val += sops->sem_op;
	}

	/* Wait for zero. */
	else
	{
		if (semaphores[semid].val > 0)
		{
			/* Do not block. */
			if (sops->sem_flg & IPC_NOWAIT)
				return (0);

			do_sleep(pid, semid, 0);
			return (pid);
		}
	}

	return (0);
}

/*============================================================================*
 * do_sem_init()                                                              *
 *============================================================================*/

/**
 * The do_sem_init() function initializes the semaphore service. It
 * traverses the table of semaphores and sleeping processes initializing
 * all entries.
 *
 * @author Pedro Henrique Penna
 */
void do_sem_init(void)
{
	/* Initialize table of semaphores. */
	for (int i = 0; i < NANVIX_SEM_MAX; i++)
	{
		semaphores[i].refcount = 0;
		semaphores[i].resource = RESOURCE_INITIALIZER;
	}

	/* Initialize table of sleeping processes. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		sleeping[i].semid = -1;
}
