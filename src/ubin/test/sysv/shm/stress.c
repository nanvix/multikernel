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

#include <nanvix/runtime/runtime.h>
#include <nanvix/ulib.h>
#include <posix/sys/stat.h>
#include <posix/sys/types.h>
#include <posix/errno.h>
#include "../../test.h"

/**
 * @brief Number of iterations for stress tests.
 */
#define NITERATIONS (2*NANVIX_SHM_MAX)

/**
 * @brief Local buffer for read/write.
 */
static char buffer[NANVIX_SHM_SIZE_MAX];

/*============================================================================*
 * Stress Test: Create / Unlink                                               *
 *============================================================================*/

/**
 * @brief Stress Test: Create / Unlink
 */
static void test_shm_create_unlink(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	for (int i = 0; i < NITERATIONS; i++)
	{
		uassert((shmid = __nanvix_shm_creat(shm_name, S_IWUSR)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);
		uassert(__nanvix_shm_unlink(shm_name) == 0);
	}
}

/*============================================================================*
 * Stress Test: Create / Unlink Overflow                                      *
 *============================================================================*/

/**
 * @brief Stress Test: Create / Unlink Overflow
 */
static void test_shm_create_unlink_overflow(void)
{
	int shmids[NANVIX_SHM_OPEN_MAX];
	char shm_name[NANVIX_SHM_NAME_MAX];

	for (int i = 0; i < NANVIX_SHM_OPEN_MAX; i++)
	{
		usprintf(shm_name, "cool-region%d", i);
		uassert((shmids[i] = __nanvix_shm_creat(shm_name, S_IWUSR)) >= 0);
	}

	usprintf(shm_name, "cool-region%d", NANVIX_SHM_OPEN_MAX);
	uassert(__nanvix_shm_creat(shm_name, S_IWUSR) == -ENFILE);

	for (int i = 0; i < NANVIX_SHM_OPEN_MAX; i++)
	{
		usprintf(shm_name, "cool-region%d", i);
		uassert(__nanvix_shm_close(shmids[i]) == 0);
		uassert(__nanvix_shm_unlink(shm_name) == 0);
	}
}

/*============================================================================*
 * Stress Test: Create Excl / Unlink                                          *
 *============================================================================*/

/**
 * @brief Stress Test: Create Excl / Unlink
 */
static void test_shm_create_excl_unlink(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	for (int i = 0; i < NITERATIONS; i++)
	{
		uassert((
			shmid = __nanvix_shm_open(
				shm_name,
				O_WRONLY | O_CREAT | O_EXCL,
				S_IWUSR)
			) >= 0
		);
		uassert(__nanvix_shm_close(shmid) == 0);
		uassert(__nanvix_shm_unlink(shm_name) == 0);
	}
}

/*============================================================================*
 * Stress Test: Create Excl / Unlink                                          *
 *============================================================================*/

/**
 * @brief Stress Test: Create Excl / Unlink Overflow
 */
static void test_shm_create_excl_unlink_overflow(void)
{
	int shmids[NANVIX_SHM_OPEN_MAX];
	char shm_name[NANVIX_SHM_NAME_MAX];

	for (int i = 0; i < NANVIX_SHM_OPEN_MAX; i++)
	{
		usprintf(shm_name, "cool-region%d", i);
		uassert((
			shmids[i] = __nanvix_shm_open(
				shm_name,
				O_WRONLY | O_CREAT | O_EXCL,
				S_IWUSR)
			) >= 0
		);
	}

	usprintf(shm_name, "cool-region%d", NANVIX_SHM_OPEN_MAX);
	uassert((
		 __nanvix_shm_open(
			shm_name,
			O_WRONLY | O_CREAT | O_EXCL,
			S_IWUSR)
		 )== -ENFILE
	);

	for (int i = 0; i < NANVIX_SHM_OPEN_MAX; i++)
	{
		usprintf(shm_name, "cool-region%d", i);
		uassert(__nanvix_shm_close(shmids[i]) == 0);
		uassert(__nanvix_shm_unlink(shm_name) == 0);
	}
}

/*============================================================================*
 * Stress Test: Read / Write                                                  *
 *============================================================================*/

/**
 * @brief Stress Test: Read / Writew
 */
static void test_shm_read_write(void)
{
	int shmids[NANVIX_SHM_OPEN_MAX];
	char shm_name[NANVIX_SHM_NAME_MAX];

	for (int i = 0; i < NANVIX_SHM_OPEN_MAX; i++)
	{
		usprintf(shm_name, "cool-region%d", i);
		uassert((shmids[i] = __nanvix_shm_creat(shm_name, S_IWUSR)) >= 0);
	}

	for (int i = 0; i < NANVIX_SHM_OPEN_MAX; i++)
	{
		uassert(__nanvix_shm_ftruncate(shmids[i], NANVIX_SHM_SIZE_MAX) == 0);

		umemset(buffer, 1, NANVIX_SHM_SIZE_MAX);
		uassert(__nanvix_shm_write(shmids[i], buffer, NANVIX_SHM_SIZE_MAX, 0) == NANVIX_SHM_SIZE_MAX);
		umemset(buffer, 0, NANVIX_SHM_SIZE_MAX);
		uassert(__nanvix_shm_read(shmids[i], buffer, NANVIX_SHM_SIZE_MAX, 0) == NANVIX_SHM_SIZE_MAX);

		/* Checksum. */
		for (size_t j = 0; j < NANVIX_SHM_SIZE_MAX; j++)
			uassert(buffer[j] == 1);
	}

	for (int i = 0; i < NANVIX_SHM_OPEN_MAX; i++)
	{
		usprintf(shm_name, "cool-region%d", i);
		uassert(__nanvix_shm_close(shmids[i]) == 0);
		uassert(__nanvix_shm_unlink(shm_name) == 0);
	}
}

/*============================================================================*
 * Stress Tests Driver Table                                                  *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_shm_stress[] = {
	{ test_shm_create_unlink,               "create/unlink              " },
	{ test_shm_create_unlink_overflow,      "create/unlink overflow     " },
	{ test_shm_create_excl_unlink,          "create_excl/unlink         " },
	{ test_shm_create_excl_unlink_overflow, "create_excl/unlink overflow" },
	{ test_shm_read_write,                  "create/unlink              " },
	{ NULL,                                  NULL                         },
};
