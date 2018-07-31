/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "test.h"

/*============================================================================*
 * API Test: Invalid Create                                                   *
 *============================================================================*/

/**
 * @brief API Test: Invalid Create
 */
static void test_ipc_mailbox_invalid_create(void)
{
	TEST_ASSERT(mailbox_create(NULL) < 0);
}

/*============================================================================*
 * API Test: Bad Create                                                       *
 *============================================================================*/

/**
 * @brief API Test: Bad Create
 */
static void *test_ipc_mailbox_bad_create_thread(void *args)
{
	((void) args);

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	TEST_ASSERT(mailbox_create("existing-name") < 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Bad Create
 */
static void test_ipc_mailbox_bad_create(void)
{
	int inbox;
	int tids[ipc_mailbox_ncores];
	pthread_t threads[ipc_mailbox_ncores];
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	TEST_ASSERT(mailbox_create("") < 0);
	TEST_ASSERT(mailbox_create(pathname) < 0);

	TEST_ASSERT((inbox = mailbox_create("existing-name")) >= 0);

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_mailbox_bad_create_thread,
			&tids[i])) == 0
		);
	}

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
		pthread_join(threads[i], NULL);

	/* House keeping. */
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Double Create                                                    *
 *============================================================================*/

/**
 * @brief API Test: Double Create
 */
static void test_ipc_mailbox_double_create(void)
{
	int inbox;

	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_create("cool-name") < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Invalid Unlink                                                   *
 *============================================================================*/

/**
 * @brief API Test: Invalid Unlink
 */
static void test_ipc_mailbox_invalid_unlink(void)
{
	TEST_ASSERT(mailbox_unlink(-1) < 0);
	TEST_ASSERT(mailbox_unlink(1000000) < 0);
}

/*============================================================================*
 * API Test: Bad Unlink                                                       *
 *============================================================================*/

/**
 * @brief API Test: Bad Unlink
 */
static void *test_ipc_mailbox_bad_unlink_thread(void *args)
{
	int tid;
	int inbox;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inbox = mailbox_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(mailbox_unlink(inbox) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Bad Unlink
 */
static void test_ipc_mailbox_bad_unlink(void)
{
	int outbox;
	int tids[ipc_mailbox_ncores];
	pthread_t threads[ipc_mailbox_ncores];

	for (int i = 0; i < NANVIX_MAILBOX_MAX; i++)
		TEST_ASSERT(mailbox_unlink(i) < 0);

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_mailbox_bad_unlink_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outbox = mailbox_open("existing-name")) >= 0);
	TEST_ASSERT(mailbox_unlink(outbox) < 0);
	TEST_ASSERT(mailbox_close(outbox) == 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Double Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Double Unlink
 */
static void test_ipc_mailbox_double_unlink(void)
{
	int inbox;

	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	TEST_ASSERT(mailbox_unlink(inbox) < 0);
}

/*============================================================================*
 * API Test: Invalid Open                                                     *
 *============================================================================*/

/**
 * @brief API Test: Invalid Open
 */
static void test_ipc_mailbox_invalid_open(void)
{
	TEST_ASSERT(mailbox_open(NULL) < 0);
}

/*============================================================================*
 * API Test: Bad Open                                                         *
 *============================================================================*/

/**
 * @brief API Test: Bad Open
 */
static void test_ipc_mailbox_bad_open(void)
{
	int inbox;
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	TEST_ASSERT(mailbox_open("") < 0);
	TEST_ASSERT(mailbox_open(pathname) < 0);
	TEST_ASSERT(mailbox_open("missing-name") < 0);
	TEST_ASSERT((inbox = mailbox_create("cool-name")) >= 0);
	TEST_ASSERT(mailbox_open("cool-name") < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Double Open                                                      *
 *============================================================================*/

/**
 * @brief API Test: Double Open
 */
static void *test_ipc_mailbox_double_open_thread(void *args)
{
	int tid;
	int inbox;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inbox = mailbox_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(mailbox_unlink(inbox) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Double Open
 */
static void test_ipc_mailbox_double_open(void)
{
	int outbox;
	int tids[ipc_mailbox_ncores];
	pthread_t threads[ipc_mailbox_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_mailbox_double_open_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outbox = mailbox_open("existing-name")) >= 0);
	TEST_ASSERT(mailbox_open("existing-name") < 0);
	TEST_ASSERT(mailbox_close(outbox) == 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Invalid Close                                                    *
 *============================================================================*/

/**
 * @brief API Test: Invalid Close
 */
static void test_ipc_mailbox_invalid_close(void)
{
	TEST_ASSERT(mailbox_close(-1) < 0);
	TEST_ASSERT(mailbox_close(1000000) < 0);
}

/*============================================================================*
 * API Test: Bad Close                                                        *
 *============================================================================*/

/**
 * @brief API Test: Bad Close
 */
static void test_ipc_mailbox_bad_close(void)
{
	int inbox;

	TEST_ASSERT(mailbox_close(0) < 0);
	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_close(inbox) < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Double Close                                                     *
 *============================================================================*/

/**
 * @brief API Test: Double Close
 */
static void *test_ipc_mailbox_double_close_thread(void *args)
{
	int tid;
	int inbox;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inbox = mailbox_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(mailbox_unlink(inbox) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Double Close
 */
static void test_ipc_mailbox_double_close(void)
{
	int outbox;
	int tids[ipc_mailbox_ncores];
	pthread_t threads[ipc_mailbox_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_mailbox_double_close_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outbox = mailbox_open("existing-name")) >= 0);
	TEST_ASSERT(mailbox_close(outbox) == 0);
	TEST_ASSERT(mailbox_close(outbox) < 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Invalid Read                                                     *
 *============================================================================*/

/**
 * @brief API Test: Invalid Read
 */
static void test_ipc_mailbox_invalid_read(void)
{
	char buffer[MAILBOX_MSG_SIZE];

	TEST_ASSERT(mailbox_read(-1, buffer, MAILBOX_MSG_SIZE) < 0);
	TEST_ASSERT(mailbox_read(1000000, buffer, MAILBOX_MSG_SIZE) < 0);
}

/*============================================================================*
 * API Test: Bad Read                                                         *
 *============================================================================*/

/**
 * @brief API Test: Bad Read
 */
static void *test_ipc_mailbox_bad_read_thread(void *args)
{
	int tid;
	int inbox;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inbox = mailbox_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(mailbox_unlink(inbox) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Bad Read
 */
static void test_ipc_mailbox_bad_read(void)
{
	int outbox;
	int tids[ipc_mailbox_ncores];
	char buffer[MAILBOX_MSG_SIZE];
	pthread_t threads[ipc_mailbox_ncores];

	for (int i = 0; i < NANVIX_MAILBOX_MAX; i++)
		TEST_ASSERT(mailbox_read(i, buffer, MAILBOX_MSG_SIZE) < 0);

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_mailbox_bad_read_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outbox = mailbox_open("existing-name")) >= 0);
	TEST_ASSERT(mailbox_read(outbox, buffer, MAILBOX_MSG_SIZE) < 0);
	TEST_ASSERT(mailbox_close(outbox) == 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Invalid Read Size                                                *
 *============================================================================*/

/**
 * @brief API Test: Invalid Read Size
 */
static void test_ipc_mailbox_invalid_read_size(void)
{
	int inbox;
	char buffer[MAILBOX_MSG_SIZE];

	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_read(inbox, buffer, -1) < 0);
	TEST_ASSERT(mailbox_read(inbox, buffer, 0) < 0);
	TEST_ASSERT(mailbox_read(inbox, buffer, MAILBOX_MSG_SIZE - 1) < 0);
	TEST_ASSERT(mailbox_read(inbox, buffer, MAILBOX_MSG_SIZE + 1) < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Null Read                                                        *
 *============================================================================*/

/**
 * @brief API Test: Null Read
 */
static void test_ipc_mailbox_null_read(void)
{
	int inbox;

	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_read(inbox, NULL, MAILBOX_MSG_SIZE) < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Invalid Write                                                    *
 *============================================================================*/

/**
 * @brief API Test: Invalid Write
 */
static void test_ipc_mailbox_invalid_write(void)
{
	char buffer[MAILBOX_MSG_SIZE];

	TEST_ASSERT(mailbox_write(-1, buffer, MAILBOX_MSG_SIZE) < 0);
	TEST_ASSERT(mailbox_write(1000000, buffer, MAILBOX_MSG_SIZE) < 0);
}

/*============================================================================*
 * API Test: Bad Write                                                        *
 *============================================================================*/

/**
 * @brief API Test: Bad Write
 */
static void test_ipc_mailbox_bad_write(void)
{
	int inbox;
	char buffer[MAILBOX_MSG_SIZE];

	TEST_ASSERT(mailbox_write(0, buffer, MAILBOX_MSG_SIZE) < 0);
	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_write(inbox, buffer, MAILBOX_MSG_SIZE) < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Invalid Write Size                                               *
 *============================================================================*/

/**
 * @brief API Test: Invalid Write Size
 */
static void *test_ipc_mailbox_invalid_write_size_thread(void *args)
{
	int tid;
	int inbox;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inbox = mailbox_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(mailbox_unlink(inbox) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Invalid Write Size
 */
static void test_ipc_mailbox_invalid_write_size(void)
{
	int outbox;
	int tids[ipc_mailbox_ncores];
	char buffer[MAILBOX_MSG_SIZE];
	pthread_t threads[ipc_mailbox_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_mailbox_invalid_write_size_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outbox = mailbox_open("existing-name")) >= 0);
	TEST_ASSERT(mailbox_write(outbox, buffer, -1) < 0);
	TEST_ASSERT(mailbox_write(outbox, buffer, 0) < 0);
	TEST_ASSERT(mailbox_write(outbox, buffer, MAILBOX_MSG_SIZE - 1) < 0);
	TEST_ASSERT(mailbox_write(outbox, buffer, MAILBOX_MSG_SIZE + 1) < 0);
	TEST_ASSERT(mailbox_close(outbox) == 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*
 * API Test: Null Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Null Write
 */
static void *test_ipc_mailbox_null_write_thread(void *args)
{
	int tid;
	int inbox;

	TEST_ASSERT(kernel_setup() == 0);
	TEST_ASSERT(runtime_setup(1) == 0);

	tid = ((int *)args)[0];

	if (tid == 1)
		TEST_ASSERT((inbox = mailbox_create("existing-name")) >= 0);

	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);

	/* House keeping. */
	if (tid == 1)
		TEST_ASSERT(mailbox_unlink(inbox) == 0);

	TEST_ASSERT(runtime_cleanup() == 0);
	TEST_ASSERT(kernel_cleanup() == 0);
	return (NULL);
}

/**
 * @brief API Test: Null Write
 */
static void test_ipc_mailbox_null_write(void)
{
	int outbox;
	int tids[ipc_mailbox_ncores];
	pthread_t threads[ipc_mailbox_ncores];

	/* Spawn driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
	{
		tids[i] = i;
		TEST_ASSERT((pthread_create(&threads[i],
			NULL,
			test_ipc_mailbox_null_write_thread,
			&tids[i])) == 0
		);
	}

	pthread_barrier_wait(&barrier);

	TEST_ASSERT((outbox = mailbox_open("existing-name")) >= 0);
	TEST_ASSERT(mailbox_write(outbox, NULL, MAILBOX_MSG_SIZE) < 0);
	TEST_ASSERT(mailbox_close(outbox) == 0);

	pthread_barrier_wait(&barrier);

	/* Wait for driver threads. */
	for (int i = 1; i < ipc_mailbox_ncores; i++)
		pthread_join(threads[i], NULL);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test ipc_mailbox_tests_fault[] = {
	{ test_ipc_mailbox_invalid_create,     "Invalid Create"     },
	{ test_ipc_mailbox_bad_create,         "Bad Create"         },
	{ test_ipc_mailbox_double_create,      "Double Create"      },
	{ test_ipc_mailbox_invalid_unlink,     "Invalid Unlink"     },
	{ test_ipc_mailbox_bad_unlink,         "Bad Unlink"         },
	{ test_ipc_mailbox_double_unlink,      "Double Unlink"      },
	{ test_ipc_mailbox_invalid_open,       "Invalid Open"       },
	{ test_ipc_mailbox_double_open,        "Double Open"        },
	{ test_ipc_mailbox_bad_open,           "Bad Open"           },
	{ test_ipc_mailbox_invalid_close,      "Invalid Close"      },
	{ test_ipc_mailbox_bad_close,          "Bad Close"          },
	{ test_ipc_mailbox_double_close,       "Double Close"       },
	{ test_ipc_mailbox_invalid_read,       "Invalid Read"       },
	{ test_ipc_mailbox_bad_read,           "Bad Read"           },
	{ test_ipc_mailbox_invalid_read_size,  "Invalid Read Size"  },
	{ test_ipc_mailbox_null_read,          "Null Read"          },
	{ test_ipc_mailbox_invalid_write,      "Invalid Write"      },
	{ test_ipc_mailbox_bad_write,          "Bad Write"          },
	{ test_ipc_mailbox_invalid_write_size, "Invalid Write Size" },
	{ test_ipc_mailbox_null_write,         "Null Write"         },
	{ NULL,                                NULL                 },
};