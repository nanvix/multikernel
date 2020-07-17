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
#include <posix/errno.h>
#include "../test.h"

/**
 * @brief Local buffer for read/write.
 */
static char buffer[NANVIX_SHM_SIZE_MAX];

/*============================================================================*
 * API Test: Create / Unlink                                                  *
 *============================================================================*/

/**
 * @brief API Test: Create / Unlink 1
 */
static void test_shm_create_unlink1(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	/* Chaning opening flags. */
	uassert((shmid = __nanvix_shm_open(shm_name, O_WRONLY | O_CREAT, S_IWUSR)) >= 0);
	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
	uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT, S_IWUSR)) >= 0);
	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
	uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR)) >= 0);
	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/**
 * @brief API Test: Create / Unlink 2
 */
static void test_shm_create_unlink2(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	/* Chaning access permission flags. */
	uassert((shmid = __nanvix_shm_creat(shm_name, S_IWUSR)) >= 0);
	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/**
 * @brief API Test: Create / Unlink
 */
static void test_shm_create_unlink(void)
{
	test_shm_create_unlink1();
	test_shm_create_unlink2();
}

/*============================================================================*
 * API Test: Exclusive Create / Unlink                                        *
 *============================================================================*/

/**
 * @brief API Test: Exclusive Create / Unlink 1
 */
static void test_shm_create_excl_unlink1(void)
{
	int shmid;
	const char *shm_name = "cool-region";


	/* Chaning opening flags. */
	uassert((shmid = __nanvix_shm_open(shm_name, O_WRONLY | O_CREAT | O_EXCL, S_IWUSR)) >= 0);
	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
	uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, S_IWUSR)) >= 0);
	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
	uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL | O_TRUNC, S_IWUSR)) >= 0);
	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/**
 * @brief API Test: Exclusive Create / Unlink 2
 */
static void test_shm_create_excl_unlink2(void)
{
	int shmid1, shmid2;
	const char *shm_name = "cool-region";

	uassert((shmid1 = __nanvix_shm_creat(shm_name, S_IWUSR)) >= 0);
	uassert((shmid2 = __nanvix_shm_open(shm_name, O_WRONLY | O_CREAT | O_EXCL, S_IWUSR)) == -EEXIST);
	uassert(__nanvix_shm_close(shmid1) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/**
 * @brief API Test: Exclusive Create / Unlink 3
 */
static void test_shm_create_excl_unlink3(void)
{
	int shmid1, shmid2;
	const char *shm_name1 = "cool-region1";
	const char *shm_name2 = "cool-region2";

	uassert((shmid1 = __nanvix_shm_creat(shm_name1, S_IWUSR)) >= 0);
	uassert((shmid2 = __nanvix_shm_open(shm_name2, O_WRONLY | O_CREAT | O_EXCL, S_IWUSR)) >= 0);
	uassert(__nanvix_shm_close(shmid2) == 0);
	uassert(__nanvix_shm_unlink(shm_name2) == 0);
	uassert(__nanvix_shm_close(shmid1) == 0);
	uassert(__nanvix_shm_unlink(shm_name1) == 0);
}


/**
 * @brief API Test: Exclusive Create / Unlink
 */
static void test_shm_create_excl_unlink(void)
{
	test_shm_create_excl_unlink1();
	test_shm_create_excl_unlink2();
	test_shm_create_excl_unlink3();
}

/*============================================================================*
 * API Test: Open / Close                                                     *
 *============================================================================*/

/**
 * @brief API Test: Open / Close 1
 */
static void test_shm_open_close1(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) >= 0);

		/* Write-only */
		uassert((shmid = __nanvix_shm_open(shm_name, O_WRONLY, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Read-only */
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDONLY, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Read and Write */
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Write-only and Truncate*/
		uassert((shmid = __nanvix_shm_open(shm_name, O_WRONLY | O_TRUNC, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Read and Write and Truncate */
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_TRUNC, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/**
 * @brief API Test: Open / Close 2
 */
static void test_shm_open_close2(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR)) >= 0);

		/* Write-only */
		uassert((shmid = __nanvix_shm_open(shm_name, O_WRONLY, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Read-only */
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDONLY, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Read and Write */
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Write-only and Truncate */
		uassert((shmid = __nanvix_shm_open(shm_name, O_WRONLY | O_TRUNC, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Read and Write and Truncate */
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_TRUNC, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/**
 * @brief API Test: Open / Close 2
 */
static void test_shm_open_close3(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) >= 0);
	uassert(__nanvix_shm_close(shmid) == 0);

		/* Write-only */
		uassert((shmid = __nanvix_shm_open(shm_name, O_WRONLY, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Read-only */
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDONLY, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Read and Write */
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Write-only */
		uassert((shmid = __nanvix_shm_open(shm_name, O_WRONLY | O_TRUNC, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

		/* Read and Write and Truncate */
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_TRUNC, 0)) >= 0);
		uassert(__nanvix_shm_close(shmid) == 0);

	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/**
 * @brief API Test: Open / Close
 */
static void test_shm_open_close(void)
{
	test_shm_open_close1();
	test_shm_open_close2();
}

/*============================================================================*
 * API Test: Truncate                                                         *
 *============================================================================*/
/**
 * @brief API Test: Truncate
 */
static void test_shm_ftruncate(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) >= 0);

		uassert(__nanvix_shm_ftruncate(shmid, 0) == 0);
		uassert(__nanvix_shm_ftruncate(shmid, NANVIX_SHM_SIZE_MAX) == 0);

	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/*============================================================================*
 * API Test: Read/Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Read/Write
 */
static void test_shm_read_write(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) >= 0);

		uassert(__nanvix_shm_ftruncate(shmid, NANVIX_SHM_SIZE_MAX) == 0);

		umemset(buffer, 1, NANVIX_SHM_SIZE_MAX);
		uassert(__nanvix_shm_write(shmid, buffer, NANVIX_SHM_SIZE_MAX, 0) == NANVIX_SHM_SIZE_MAX);
		umemset(buffer, 0, NANVIX_SHM_SIZE_MAX);
		uassert(__nanvix_shm_read(shmid, buffer, NANVIX_SHM_SIZE_MAX, 0) == NANVIX_SHM_SIZE_MAX);

		/* Checksum. */
		for (size_t i = 0; i < NANVIX_SHM_SIZE_MAX; i++)
			uassert(buffer[i] == 1);

	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/*============================================================================*
 * API Test: Inval                                                             *
 *============================================================================*/

/**
 * @brief API Test: Inval
 */
static void test_shm_inval(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) >= 0);

		uassert(__nanvix_shm_ftruncate(shmid, NANVIX_SHM_SIZE_MAX) == 0);

		umemset(buffer, 1, NANVIX_SHM_SIZE_MAX);
		uassert(__nanvix_shm_write(shmid, buffer, NANVIX_SHM_SIZE_MAX, 0) == NANVIX_SHM_SIZE_MAX);
		umemset(buffer, 0, NANVIX_SHM_SIZE_MAX);
		uassert(__nanvix_shm_read(shmid, buffer, NANVIX_SHM_SIZE_MAX, 0) == NANVIX_SHM_SIZE_MAX);

		/* Checksum. */
		for (size_t i = 0; i < NANVIX_SHM_SIZE_MAX; i++)
			uassert(buffer[i] == 1);

		uassert(__nanvix_shm_inval(shmid) == 0);

	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/*============================================================================*
 * API Test Driver Table                                                      *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_shm_api[] = {
	{ test_shm_create_unlink,      "create/unlink     " },
	{ test_shm_create_excl_unlink, "create_excl/unlink" },
	{ test_shm_open_close,         "open/close        " },
	{ test_shm_ftruncate,          "ftruncate         " },
	{ test_shm_read_write,         "read/write        " },
	{ test_shm_inval,              "inval             " },
	{ NULL,                         NULL                }
};
