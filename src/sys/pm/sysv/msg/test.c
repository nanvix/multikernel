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
#define __SYSV_SERVER

#include <nanvix/servers/sysv.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/*============================================================================*
 * API Tests                                                                  *
 *============================================================================*/

/**
 * @brief API Test: Get / Close
 */
static void test_api_msg_get_close(void)
{
	int msgid;

	uassert((msgid = do_msg_get(100, IPC_CREAT)) >= 0);
	uassert(do_msg_close(msgid) == 0);

	uassert((msgid = do_msg_get(100, IPC_CREAT)) >= 0);
	uassert(do_msg_get(100, 0) == msgid);
	uassert(do_msg_close(msgid) == 0);
	uassert(do_msg_close(msgid) == 0);
}

/**
 * @brief API Test: Send / Receive
 */
static void test_api_msg_send_receive(void)
{
	int msgid;
	void *msgp;

	uassert((msgid = do_msg_get(100, IPC_CREAT)) >= 0);

	uassert(do_msg_send(msgid, &msgp, NANVIX_MSG_SIZE_MAX, IPC_NOWAIT) == 0);
	umemset(msgp, 1, NANVIX_MSG_SIZE_MAX);
	uassert(do_msg_receive(msgid, &msgp, NANVIX_MSG_SIZE_MAX, 0, IPC_NOWAIT) == 0);

	/* Check sum. */
	for (int i = 0; i < NANVIX_MSG_SIZE_MAX; i++)
		uassert(((char *)msgp)[i] == 1);

	uassert(do_msg_close(msgid) == 0);
}

/*============================================================================*
 * Fault Injection Tests                                                      *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Get
 */
static void test_fault_msg_get_invalid(void)
{
	/* TODO: implement. */
}

/**
 * @brief Fault Injection Test: Bad Get
 */
static void test_fault_msg_get_bad(void)
{
	/* TODO: implement. */
}

/**
 * @brief Fault Injection Test: Invalid Close
 */
static void test_fault_msg_close_invalid(void)
{
	uassert(do_msg_close(-1) == -EINVAL);
	uassert(do_msg_close(NANVIX_MSG_MAX) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Bad Close
 */
static void test_fault_msg_close_bad(void)
{
	uassert(do_msg_close(0) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Invalid Send
 */
static void test_fault_msg_send_invalid(void)
{
	int msgid;
	void *msgp;

	uassert(do_msg_send(-1, &msgp, NANVIX_MSG_SIZE_MAX, IPC_NOWAIT) == -EINVAL);
	uassert(do_msg_send(NANVIX_MSG_MAX, &msgp, NANVIX_MSG_SIZE_MAX, IPC_NOWAIT) == -EINVAL);

	uassert((msgid = do_msg_get(100, IPC_CREAT)) >= 0);
	uassert(do_msg_send(msgid, NULL, NANVIX_MSG_SIZE_MAX, IPC_NOWAIT) == -EINVAL);
	uassert(do_msg_send(msgid, &msgp, 1, IPC_NOWAIT) == -EINVAL);
	uassert(do_msg_close(msgid) == 0);
}

/**
 * @brief Fault Injection Test: Bad Send
 */
static void test_fault_msg_send_bad(void)
{
	void *msgp;
	uassert(do_msg_send(0, &msgp, NANVIX_MSG_SIZE_MAX, IPC_NOWAIT) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Invalid Receive
 */
static void test_fault_msg_receive_invalid(void)
{
	int msgid;
	void *msgp;

	uassert(do_msg_receive(-1, &msgp, NANVIX_MSG_SIZE_MAX, 0, IPC_NOWAIT) == -EINVAL);
	uassert(do_msg_receive(NANVIX_MSG_MAX, &msgp, NANVIX_MSG_SIZE_MAX, 0, IPC_NOWAIT) == -EINVAL);

	uassert((msgid = do_msg_get(100, IPC_CREAT)) >= 0);
	uassert(do_msg_receive(msgid, NULL, NANVIX_MSG_SIZE_MAX, 0, IPC_NOWAIT) == -EINVAL);
	uassert(do_msg_receive(msgid, &msgp, 1, 0, IPC_NOWAIT) == -EINVAL);
	uassert(do_msg_close(msgid) == 0);
}

/**
 * @brief Fault Injection Test: Bad Receive
 */
static void test_fault_msg_receive_bad(void)
{
	void *msgp;
	uassert(do_msg_receive(0, &msgp, NANVIX_MSG_SIZE_MAX, 0, IPC_NOWAIT) == -EINVAL);
}

/*============================================================================*
 * Stress Tests                                                               *
 *============================================================================*/

/**
 * @brief Stress Test: Get / Close 1
 */
static void test_stress_msg_get_close1(void)
{
	int msgid;

	for (int i = 0; i < NANVIX_MSG_MAX; i++)
	{
		uassert((msgid = do_msg_get(100 + i, IPC_CREAT)) >= 0);
		uassert(do_msg_close(msgid) == 0);
	}
}

/**
 * @brief Stress Test: Get / Close 2
 */
static void test_stress_msg_get_close2(void)
{
	int msgids[NANVIX_MSG_MAX];

	for (int i = 0; i < NANVIX_MSG_MAX; i++)
		uassert((msgids[i] = do_msg_get(100 + i, IPC_CREAT)) >= 0);

	for (int i = 0; i < NANVIX_MSG_MAX; i++)
		uassert(do_msg_close(msgids[i]) == 0);
}

/**
 * @brief Stress Test: Send / Receive 1
 */
static void test_stress_msg_send_receive1(void)
{
	int msgid;
	void *msgp;

	uassert((msgid = do_msg_get(100, IPC_CREAT)) >= 0);

		for (int i = 0; i < 2*NANVIX_MSG_LENGTH_MAX; i++)
		{
			uassert(do_msg_send(msgid, &msgp, NANVIX_MSG_SIZE_MAX, IPC_NOWAIT) == 0);
			umemset(msgp, i, NANVIX_MSG_SIZE_MAX);

			uassert(do_msg_receive(msgid, &msgp, NANVIX_MSG_SIZE_MAX, 0, IPC_NOWAIT) == 0);

			/* Check sum. */
			for (int j = 0; j < NANVIX_MSG_SIZE_MAX; j++)
				uassert(((char *)msgp)[j] == i);
		}

	uassert(do_msg_close(msgid) == 0);
}

/**
 * @brief Stress Test: Send / Receive 2
 */
static void test_stress_msg_send_receive2(void)
{
	int msgid;
	void *msgp;

	uassert((msgid = do_msg_get(100, IPC_CREAT)) >= 0);

		for (int i = 0; i < NANVIX_MSG_LENGTH_MAX; i++)
		{
			uassert(do_msg_send(msgid, &msgp, NANVIX_MSG_SIZE_MAX, IPC_NOWAIT) == 0);
			umemset(msgp, i, NANVIX_MSG_SIZE_MAX);
		}

		for (int i = 0; i < NANVIX_MSG_LENGTH_MAX; i++)
		{
			uassert(do_msg_receive(msgid, &msgp, NANVIX_MSG_SIZE_MAX, 0, IPC_NOWAIT) == 0);

			/* Check sum. */
			for (int j = 0; j < NANVIX_MSG_SIZE_MAX; j++)
				uassert(((char *)msgp)[j] == i);
		}

	uassert(do_msg_close(msgid) == 0);
}

/*============================================================================*
 * Test Driver                                                                *
 *============================================================================*/

/**
 * @brief Virtual File System Tests
 */
static struct
{
	void (*func)(void); /**< Test Function */
	const char *name;   /**< Test Name     */
} msg_tests[] = {
	{ test_api_msg_get_close,         "[api] get close        " },
	{ test_api_msg_send_receive,      "[api] send receive     " },
	{ test_fault_msg_get_invalid,     "[fault] invalid get    " },
	{ test_fault_msg_get_bad,         "[fault] bad get        " },
	{ test_fault_msg_close_invalid,   "[fault] invalid close  " },
	{ test_fault_msg_close_bad,       "[fault] bad close      " },
	{ test_fault_msg_send_invalid,    "[fault] invalid send   " },
	{ test_fault_msg_send_bad,        "[fault] bad send       " },
	{ test_fault_msg_receive_invalid, "[fault] invalid receive" },
	{ test_fault_msg_receive_bad,     "[fault] bad receive    " },
	{ test_stress_msg_get_close1,     "[stress] get close 1   " },
	{ test_stress_msg_get_close2,     "[stress] get close 2   " },
	{ test_stress_msg_send_receive1,  "[stress] send receive 1" },
	{ test_stress_msg_send_receive2,  "[stress] send receive 2" },
	{ NULL,                           NULL                      },
};

/**
 * @brief Runs regression tests on message queues.
 */
void msg_test(void)
{
	for (int i = 0; msg_tests[i].func != NULL; i++)
	{
		msg_tests[i].func();

		uprintf("[nanvix][sysv][msg]%s passed", msg_tests[i].name);
	}
}

