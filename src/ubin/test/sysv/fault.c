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

/**
 * Used for tests.
 */
char msgp[NANVIX_MSG_SIZE_MAX];

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
	uassert(__nanvix_msg_close(-1) == -EINVAL);
	uassert(__nanvix_msg_close(NANVIX_MSG_MAX) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Bad Close
 */
static void test_fault_msg_close_bad(void)
{
	uassert(__nanvix_msg_close(0) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Invalid Send
 */
static void test_fault_msg_send_invalid(void)
{
	int msgid;

	uassert(__nanvix_msg_send(-1, msgp, NANVIX_MSG_SIZE_MAX, 0) == -EINVAL);
	uassert(__nanvix_msg_send(NANVIX_MSG_MAX, msgp, NANVIX_MSG_SIZE_MAX, 0) == -EINVAL);

	uassert((msgid = __nanvix_msg_get(100, 0)) >= 0);
	uassert(__nanvix_msg_send(msgid, NULL, NANVIX_MSG_SIZE_MAX, 0) == -EINVAL);
	uassert(__nanvix_msg_send(msgid, msgp, 1, 0) == -EINVAL);
	uassert(__nanvix_msg_close(msgid) == 0);
}

/**
 * @brief Fault Injection Test: Bad Send
 */
static void test_fault_msg_send_bad(void)
{
	uassert(__nanvix_msg_send(0, msgp, NANVIX_MSG_SIZE_MAX, 0) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Invalid Receive
 */
static void test_fault_msg_receive_invalid(void)
{
	int msgid;

	uassert(__nanvix_msg_receive(-1, msgp, NANVIX_MSG_SIZE_MAX, 0, 0) == -EINVAL);
	uassert(__nanvix_msg_receive(NANVIX_MSG_MAX, msgp, NANVIX_MSG_SIZE_MAX, 0, 9) == -EINVAL);

	uassert((msgid = __nanvix_msg_get(100, 0)) >= 0);
	uassert(__nanvix_msg_receive(msgid, NULL, NANVIX_MSG_SIZE_MAX, 0, 0) == -EINVAL);
	uassert(__nanvix_msg_receive(msgid, msgp, 1, 0, 0) == -EINVAL);
	uassert(__nanvix_msg_close(msgid) == 0);
}

/**
 * @brief Fault Injection Test: Bad Receive
 */
static void test_fault_msg_receive_bad(void)
{
	uassert(__nanvix_msg_receive(0, msgp, NANVIX_MSG_SIZE_MAX, 0, 0) == -EINVAL);
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
	{ test_fault_msg_get_invalid,     "[fault] invalid get    " },
	{ test_fault_msg_get_bad,         "[fault] bad get        " },
	{ test_fault_msg_close_invalid,   "[fault] invalid close  " },
	{ test_fault_msg_close_bad,       "[fault] bad close      " },
	{ test_fault_msg_send_invalid,    "[fault] invalid send   " },
	{ test_fault_msg_send_bad,        "[fault] bad send       " },
	{ test_fault_msg_receive_invalid, "[fault] invalid receive" },
	{ test_fault_msg_receive_bad,     "[fault] bad receive    " },
	{ NULL,                           NULL                      },
};
