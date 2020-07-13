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
#include <nanvix/sys/noc.h>
#include <posix/sys/types.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/*============================================================================*
 * API Tests                                                                  *
 *============================================================================*/

/**
 * @brief API Test: Get / Close
 */
static void test_api_sem_get_close(void)
{
	int semid;

	uassert((semid = do_sem_get(100, 0)) >= 0);
	uassert(do_sem_close(semid) == 0);

	uassert((semid = do_sem_get(100, 0)) >= 0);
	uassert(do_sem_get(100, 0) == semid);
	uassert(do_sem_close(semid) == 0);
	uassert(do_sem_close(semid) == 0);
}

/**
 * @brief API Test: Up / Down
 */
static void test_api_sem_up_down(void)
{
	int semid;
	nanvix_pid_t pid;
	struct nanvix_sembuf sembuf;

	pid = kcluster_get_num();

	uassert((semid = do_sem_get(100, 0)) >= 0);

	sembuf.sem_op = 1;
	uassert(do_sem_operate(pid, semid, &sembuf) == 0);
	sembuf.sem_op = -1;
	uassert(do_sem_operate(pid, semid, &sembuf) == 0);

	uassert(do_sem_close(semid) == 0);
}

/*============================================================================*
 * Fault Injection Tests                                                      *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Get
 */
static void test_fault_sem_get_invalid(void)
{
	/* TODO: implement. */
}

/**
 * @brief Fault Injection Test: Bad Get
 */
static void test_fault_sem_get_bad(void)
{
	/* TODO: implement. */
}

/**
 * @brief Fault Injection Test: Invalid Close
 */
static void test_fault_sem_close_invalid(void)
{
	uassert(do_sem_close(-1) == -EINVAL);
	uassert(do_sem_close(NANVIX_SEM_MAX) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Bad Close
 */
static void test_fault_sem_close_bad(void)
{
	uassert(do_sem_close(0) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Invalid Operate
 */
static void test_fault_sem_operate_invalid(void)
{
	int semid;
	nanvix_pid_t pid;
	struct nanvix_sembuf sembuf;

	pid = kcluster_get_num();

	uassert(do_sem_operate(pid, -1, &sembuf) == -EINVAL);

	uassert((semid = do_sem_get(100, 0)) >= 0);
	uassert(do_sem_operate(-1, semid, &sembuf) == -EINVAL);
	uassert(do_sem_operate(pid, semid, NULL) == -EINVAL);
	uassert(do_sem_close(semid) == 0);
}

/**
 * @brief Fault Injection Test: Bad Operate
 */
static void test_fault_sem_operate_bad(void)
{
	nanvix_pid_t pid;
	struct nanvix_sembuf sembuf;

	pid = kcluster_get_num();

	uassert(do_sem_operate(pid, 0, &sembuf) == -EINVAL);
}

/*============================================================================*
 * Stress Tests                                                               *
 *============================================================================*/

/**
 * @brief Stress Test: Get / Close 1
 */
static void test_stress_sem_get_close1(void)
{
	int semid;

	for (int i = 0; i < NANVIX_SEM_MAX; i++)
	{
		uassert((semid = do_sem_get(100, 0)) >= 0);
		uassert(do_sem_close(semid) == 0);
	}
}

/**
 * @brief Stress Test: Get / Close 2
 */
static void test_stress_sem_get_close2(void)
{
	int semids[NANVIX_SEM_MAX];

	for (int i = 0; i < NANVIX_SEM_MAX; i++)
		uassert((semids[i] = do_sem_get(100 + i, 0)) >= 0);

	for (int i = 0; i < NANVIX_SEM_MAX; i++)
		uassert(do_sem_close(semids[i]) == 0);
}

/**
 * @brief Stress Test: Up / Down 1
 */
static void test_stress_sem_up_down1(void)
{
	nanvix_pid_t pid;
	int semid;
	struct nanvix_sembuf sembuf;

	pid = kcluster_get_num();

	uassert((semid = do_sem_get(100, 0)) >= 0);

		for (int i = 0; i < NANVIX_SEM_MAX; i++)
		{
			sembuf.sem_op = 1;
			uassert(do_sem_operate(pid, semid, &sembuf) == 0);

			sembuf.sem_op = -1;
			uassert(do_sem_operate(pid, semid, &sembuf) == 0);
		}

	uassert(do_sem_close(semid) == 0);
}

/**
 * @brief Stress Test: Up / Down 2
 */
static void test_stress_sem_up_down2(void)
{
	nanvix_pid_t pid;
	int semid;
	struct nanvix_sembuf sembuf;

	pid = kcluster_get_num();

	uassert((semid = do_sem_get(100, 0)) >= 0);

		for (int i = 0; i < NANVIX_SEM_MAX; i++)
		{
			sembuf.sem_op = 1;
			uassert(do_sem_operate(pid, semid, &sembuf) == 0);
		}

		for (int i = 0; i < NANVIX_SEM_MAX; i++)
		{
			sembuf.sem_op = -1;
			uassert(do_sem_operate(pid, semid, &sembuf) == 0);
		}

	uassert(do_sem_close(semid) == 0);
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
} sem_tests[] = {
	{ test_api_sem_get_close,         "[api] get close         " },
	{ test_api_sem_up_down,           "[api] up down           " },
	{ test_fault_sem_get_invalid,     "[fault] invalid get     " },
	{ test_fault_sem_get_bad,         "[fault] bad get         " },
	{ test_fault_sem_close_invalid,   "[fault] invalid close   " },
	{ test_fault_sem_close_bad,       "[fault] bad close       " },
	{ test_fault_sem_operate_invalid, "[fault] invalid operate " },
	{ test_fault_sem_operate_bad,     "[fault] bad operate     " },
	{ test_stress_sem_get_close1,     "[stress] get close 1    " },
	{ test_stress_sem_get_close2,     "[stress] get close 2    " },
	{ test_stress_sem_up_down1,       "[stress] up down 1      " },
	{ test_stress_sem_up_down2,       "[stress] up down 2      " },
	{ NULL,                           NULL                       },
};

/**
 * @brief Runs regression tests on message queues.
 */
void sem_test(void)
{
	for (int i = 0; sem_tests[i].func != NULL; i++)
	{
		sem_tests[i].func();

		uprintf("[nanvix][sysv][sem]%s passed", sem_tests[i].name);
	}
}

