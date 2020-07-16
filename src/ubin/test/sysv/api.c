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

#include <nanvix/runtime/pm.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/sys/ipc.h>

/*============================================================================*
 * Message Queue                                                              *
 *============================================================================*/

/**
 * Used for tests.
 */
static char msgp[NANVIX_MSG_SIZE_MAX];

/**
 * @brief API Test: Get / Close
 */
static void test_api_msg_get_close(void)
{
	int msgid;

	uassert((msgid = __nanvix_msg_get(100, 0)) >= 0);
	uassert(__nanvix_msg_close(msgid) == 0);

	uassert((msgid = __nanvix_msg_get(100, 0)) >= 0);
	uassert(__nanvix_msg_get(100, 0) == msgid);
	uassert(__nanvix_msg_close(msgid) == 0);
	uassert(__nanvix_msg_close(msgid) == 0);
}

/**
 * @brief API Test: Send / Receive
 */
static void test_api_msg_send_receive(void)
{
	int msgid;

	uassert((msgid = __nanvix_msg_get(100, 0)) >= 0);

	umemset(msgp, 1, NANVIX_MSG_SIZE_MAX);
	uassert(__nanvix_msg_send(msgid, msgp, NANVIX_MSG_SIZE_MAX, 0) == 0);
	umemset(msgp, 0, NANVIX_MSG_SIZE_MAX);
	uassert(__nanvix_msg_receive(msgid, msgp, NANVIX_MSG_SIZE_MAX, 0, 0) == 0);

	/* Check sum. */
	for (int i = 0; i < NANVIX_MSG_SIZE_MAX; i++)
			uassert(msgp[i] == 1);

	uassert(__nanvix_msg_close(msgid) == 0);
}

/*============================================================================*
 * Semaphores                                                                 *
 *============================================================================*/

/**
 * @brief API Test: Get / Close
 */
static void test_api_sem_get_close(void)
{
	int semid;

	/* Create. */
	uassert((semid = __nanvix_semget(100, IPC_CREAT)) >= 0);
	uassert(__nanvix_sem_close(semid) == 0);

	/* Create exclusive. */
	uassert((semid = __nanvix_semget(100, IPC_CREAT)) >= 0);
	uassert(__nanvix_semget(100, IPC_CREAT | IPC_EXCL) == -EEXIST);
	uassert(__nanvix_sem_close(semid) == 0);

	/* Open. */
	uassert((semid = __nanvix_semget(100, IPC_CREAT)) >= 0);
	uassert(__nanvix_semget(100, 0) == semid);
	uassert(__nanvix_sem_close(semid) == 0);
	uassert(__nanvix_sem_close(semid) == 0);
}

/**
 * @brief API Test: Send / Receive
 */
static void test_api_sem_up_down(void)
{
	int semid;
	struct nanvix_sembuf sembuf;

	uassert((semid = __nanvix_semget(100, IPC_CREAT | IPC_EXCL)) >= 0);

		sembuf.sem_op = 1;
		uassert(__nanvix_semop(semid, &sembuf, 1) == 0);

		sembuf.sem_op = -1;
		uassert(__nanvix_semop(semid, &sembuf, 1) == 0);

		sembuf.sem_op = 0;
		uassert(__nanvix_semop(semid, &sembuf, 1) == 0);

		sembuf.sem_op = -1;
		sembuf.sem_flg = IPC_NOWAIT;
		uassert(__nanvix_semop(semid, &sembuf, 1) == 0);

		sembuf.sem_op = 0;
		sembuf.sem_flg = IPC_NOWAIT;
		uassert(__nanvix_semop(semid, &sembuf, 1) == 0);

	uassert(__nanvix_sem_close(semid) == 0);
}

/*============================================================================*
 * Test Driver                                                                *
 *============================================================================*/

/**
 * @brief Virtual File System Tests
 */
struct
{
	void (*func)(void); /**< Test Function */
	const char *name;   /**< Test Name     */
} tests_sysv_api[] = {
	{ test_api_msg_get_close,     "[msg][api] get close    " },
	{ test_api_msg_send_receive,  "[msg][api] send receive " },
	{ test_api_sem_get_close,     "[sem][api] get close    " },
	{ test_api_sem_up_down,       "[sem][api] up down      " },
	{ NULL,                        NULL                      },
};
