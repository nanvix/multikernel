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

/* Must come first. */
#define __NEED_SYSV_SERVER
#define __SYSV_SERVICE

#include <nanvix/runtime/runtime.h>
#include <nanvix/config.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>

/*============================================================================*
 * __nanvix_sem_get()                                                         *
 *============================================================================*/

/**
 * The __do_nanvix_semget() function retrieves the semaphore that
 * matches the @p key parameter. The @p semflg flags specifies actions
 * to take during this operation. Upon successful completo, the ID of
 * the target semaphore is returned.
 */
static int __do_nanvix_semget(key_t key, int semflg)
{
	struct sysv_message sem;

	/* Client not initialized. */
	if (!__nanvix_sysv_is_initialized())
		return (-EAGAIN);

	/* Build message. */
	message_header_build(&sem.header, SYSV_SEM_GET);
	sem.payload.sem.get.key = key;
	sem.payload.sem.get.semflg = semflg;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			__nanvix_sysv_outbox(),
			&sem,
			sizeof(struct sysv_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&sem,
			sizeof(struct sysv_message)
		) == sizeof(struct sysv_message)
	);

	if (sem.payload.ret.status < 0)
		return (sem.payload.ret.status);

	return (sem.payload.ret.ipcid);
}

/**
 * @see __do_nanvix_semget().
 */
int __nanvix_semget(key_t key, int semflg)
{
	/* TODO: check parameters. */

	return (__do_nanvix_semget(key, semflg));
}

/*============================================================================*
 * __nanvix_sem_close()                                                       *
 *============================================================================*/

/**
 * The __do_nanvix_sem_close() function  closes the semaphore identified
 * by @p semid. Upon successful completo, zero is returned.
 */
int __do_nanvix_sem_close(int semid)
{
	struct sysv_message sem;

	/* Client not initialized. */
	if (!__nanvix_sysv_is_initialized())
		return (-EAGAIN);

	/* Build message. */
	message_header_build(&sem.header, SYSV_SEM_CLOSE);
	sem.payload.sem.close.semid = semid;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			__nanvix_sysv_outbox(),
			&sem,
			sizeof(struct sysv_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&sem,
			sizeof(struct sysv_message)
		) == sizeof(struct sysv_message)
	);

	return (sem.payload.ret.status);
}

/**
 * @see __do_nanvix_sem_close().
 */
int __nanvix_sem_close(int semid)
{
	/* Invalid ID for semaphore. */
	if (!semid_is_valid(semid))
		return (-EINVAL);

	return (__do_nanvix_sem_close(semid));
}

/*============================================================================*
 * __nanvix_semop()                                                           *
 *============================================================================*/

/**
 * The __do_nanvix_semop() function operates on the semaphore identied
 * by @p semid. The @p sops parameter speficies the operation to
 * execute.  Upon successful completion, this function returns either:
 * (i) zero (indicating tha the operation was promptly completed and no
 * futher action is required; or (ii) a positive process identifier that
 * signals, which in turn provides additional information on whether the
 * calling process should block (return value equals @p pid parameter)
 * or another process should be awaken return value differs from @p pid
 * parameter).
 */
int __do_nanvix_semop(int semid, const struct nanvix_sembuf *sops, size_t nsops)
{
	struct sysv_message sem;

	/* Client not initialized. */
	if (!__nanvix_sysv_is_initialized())
		return (-EAGAIN);

	((void) nsops);

	/* Build message. */
	message_header_build(&sem.header, SYSV_SEM_OPERATE);
	sem.payload.sem.operate.semid = semid;
	umemcpy(&sem.payload.sem.operate.sembuf, sops, sizeof(struct nanvix_sembuf));

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			__nanvix_sysv_outbox(),
			&sem,
			sizeof(struct sysv_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&sem,
			sizeof(struct sysv_message)
		) == sizeof(struct sysv_message)
	);

	return (sem.payload.ret.status);
}

/**
 * @see __do_nanvix_semop().
 */
int __nanvix_semop(int semid, const struct nanvix_sembuf *sops, size_t nsops)
{
	/* Invalid ID for semaphore. */
	if (!semid_is_valid(semid))
		return (-EINVAL);

	/* Invalid message. */
	if (sops == NULL)
		return (-EINVAL);

	/* Invalid array length. */
	if (nsops != 1)
		return (-EINVAL);

	/* TODO: check parameters. */

	return (__do_nanvix_semop(semid, sops, nsops));
}
