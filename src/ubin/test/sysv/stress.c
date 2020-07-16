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

/*============================================================================*
 * Message Queue                                                              *
 *============================================================================*/

/**
 * Used for tests.
 */
char msgp[NANVIX_MSG_SIZE_MAX];

/**
 * @brief Stress Test: Get / Close 1
 */
static void test_stress_msg_get_close1(void)
{
	int msgid;

	for (int i = 0; i < NANVIX_MSG_MAX; i++)
	{
		uassert((msgid = __nanvix_msg_get(100, 0)) >= 0);
		uassert(__nanvix_msg_close(msgid) == 0);
	}
}

/**
 * @brief Stress Test: Get / Close 2
 */
static void test_stress_msg_get_close2(void)
{
	int msgids[NANVIX_MSG_MAX];

	for (int i = 0; i < NANVIX_MSG_MAX; i++)
		uassert((msgids[i] = __nanvix_msg_get(100 + i, 0)) >= 0);

	for (int i = 0; i < NANVIX_MSG_MAX; i++)
		uassert(__nanvix_msg_close(msgids[i]) == 0);
}

/**
 * @brief Stress Test: Send / Receive 1
 */
static void test_stress_msg_send_receive1(void)
{
	int msgid;

	uassert((msgid = __nanvix_msg_get(100, 0)) >= 0);

		for (int i = 0; i < 2*NANVIX_MSG_LENGTH_MAX; i++)
		{
			umemset(msgp, i, NANVIX_MSG_SIZE_MAX);
			uassert(__nanvix_msg_send(msgid, msgp, NANVIX_MSG_SIZE_MAX, 0) == 0);

			umemset(msgp, 0, NANVIX_MSG_SIZE_MAX);
			uassert(__nanvix_msg_receive(msgid, msgp, NANVIX_MSG_SIZE_MAX, 0, 0) == 0);

			/* Check sum. */
			for (int j = 0; j < NANVIX_MSG_SIZE_MAX; j++)
				uassert(((char *)msgp)[j] == i);
		}

	uassert(__nanvix_msg_close(msgid) == 0);
}

/**
 * @brief Stress Test: Send / Receive 2
 */
static void test_stress_msg_send_receive2(void)
{
	int msgid;

	uassert((msgid = __nanvix_msg_get(100, 0)) >= 0);

		for (int i = 0; i < NANVIX_MSG_LENGTH_MAX; i++)
		{
			umemset(msgp, i, NANVIX_MSG_SIZE_MAX);
			uassert(__nanvix_msg_send(msgid, msgp, NANVIX_MSG_SIZE_MAX, 0) == 0);
		}

		for (int i = 0; i < NANVIX_MSG_LENGTH_MAX; i++)
		{
			umemset(msgp, 0, NANVIX_MSG_SIZE_MAX);
			uassert(__nanvix_msg_receive(msgid, msgp, NANVIX_MSG_SIZE_MAX, 0, 0) == 0);

			/* Check sum. */
			for (int j = 0; j < NANVIX_MSG_SIZE_MAX; j++)
				uassert(msgp[j] == i);
		}

	uassert(__nanvix_msg_close(msgid) == 0);
}

/*============================================================================*
 * Semaphores                                                                 *
 *============================================================================*/

/**
 * @brief Stress Test: Get / Close 1
 */
static void test_stress_sem_get_close1(void)
{
	int semid;

	for (int i = 0; i < NANVIX_SEM_MAX; i++)
	{
		uassert((semid = __nanvix_semget(100 + i, IPC_CREAT | IPC_EXCL)) >= 0);
		uassert(__nanvix_sem_close(semid) == 0);
	}
}

/**
 * @brief Stress Test: Get / Close 2
 */
static void test_stress_sem_get_close2(void)
{
	int semids[NANVIX_SEM_MAX];

	for (int i = 0; i < NANVIX_SEM_MAX; i++)
		uassert((semids[i] = __nanvix_semget(100 + i, IPC_CREAT | IPC_EXCL)) >= 0);

	for (int i = 0; i < NANVIX_SEM_MAX; i++)
		uassert(__nanvix_sem_close(semids[i]) == 0);
}

/**
 * @brief Stress Test: Up / Down 1
 */
static void test_stress_sem_up_down1(void)
{
	int semid;
	struct nanvix_sembuf sembuf;

	uassert((semid = __nanvix_semget(100, IPC_CREAT | IPC_EXCL)) >= 0);

		for (int i = 0; i < NANVIX_SEM_MAX; i++)
		{
			sembuf.sem_op = 1;
			uassert(__nanvix_semop(semid, &sembuf, 1) == 0);

			sembuf.sem_op = -1;
			uassert(__nanvix_semop(semid, &sembuf, 1) == 0);
		}

	uassert(__nanvix_sem_close(semid) == 0);
}

/**
 * @brief Stress Test: Up / Down 2
 */
static void test_stress_sem_up_down2(void)
{
	int semid;
	struct nanvix_sembuf sembuf;

	uassert((semid = __nanvix_semget(100, IPC_CREAT | IPC_EXCL)) >= 0);

		for (int i = 0; i < NANVIX_SEM_MAX; i++)
		{
			sembuf.sem_op = 1;
			uassert(__nanvix_semop(semid, &sembuf, 1) == 0);
		}

		for (int i = 0; i < NANVIX_SEM_MAX; i++)
		{
			sembuf.sem_op = -1;
			uassert(__nanvix_semop(semid, &sembuf, 1) == 0);
		}

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
} tests_sysv_stress[] = {
	{ test_stress_msg_get_close1,     "[msg][stress] get close 1    " },
	{ test_stress_msg_get_close2,     "[msg][stress] get close 2    " },
	{ test_stress_msg_send_receive1,  "[msg][stress] send receive 1 " },
	{ test_stress_msg_send_receive2,  "[msg][stress] send receive 2 " },
	{ test_stress_sem_get_close1,     "[sem][stress] get close 1    " },
	{ test_stress_sem_get_close2,     "[sem][stress] get close 2    " },
	{ test_stress_sem_up_down1,       "[sem][stress] up down 1      " },
	{ test_stress_sem_up_down2,       "[sem][stress] up down 2      " },
	{ NULL,                           NULL                            },
};
