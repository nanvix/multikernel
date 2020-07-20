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
 * @brief Length of a long name for a shared memory region.
 */
#define SHM_LONG_NAME_LEN (2*NANVIX_SHM_NAME_MAX)

/**
 * @brief Local buffer for read/write.
 */
static char buffer[NANVIX_SHM_SIZE_MAX];

/*============================================================================*
 * Fault Injection Test: Invalid Create                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Create 1
 */
static void test_shm_create_inval1(void)
{
	mode_t modeinval;
	char longname[SHM_LONG_NAME_LEN];

	modeinval = ~(S_IRUSR | S_IWUSR);

	for (size_t i = 0; i < SHM_LONG_NAME_LEN; i++)
		longname[i] = 'a';
	longname[SHM_LONG_NAME_LEN - 1] = '\0';

	/* Invalid name. */
	uassert(__nanvix_shm_creat(NULL, S_IWUSR) == -EINVAL);
	uassert(__nanvix_shm_creat("", S_IWUSR) == -EINVAL);
	uassert(__nanvix_shm_creat(longname, S_IWUSR) == -ENAMETOOLONG);

	/* Invalid mode. */
	uassert(__nanvix_shm_creat("cool-name", modeinval) == -ENOTSUP);
}

/**
 * @brief Fault Injection Test: Invalid Create 2
 */
static void test_shm_create_inval2(void)
{
	mode_t modeinval;
	char longname[SHM_LONG_NAME_LEN];

	modeinval = ~(S_IRUSR | S_IWUSR);

	for (size_t i = 0; i < SHM_LONG_NAME_LEN; i++)
		longname[i] = 'a';
	longname[SHM_LONG_NAME_LEN - 1] = '\0';

	/* Invalid name. */
	uassert(__nanvix_shm_open(NULL, O_WRONLY | O_CREAT, S_IWUSR) == -EINVAL);
	uassert(__nanvix_shm_open("", O_WRONLY | O_CREAT, S_IWUSR) == -EINVAL);
	uassert(__nanvix_shm_open(longname, O_WRONLY | O_CREAT, S_IWUSR) == -ENAMETOOLONG);

	/* Invalid opening flag. */
	uassert(__nanvix_shm_open("cool-name", O_RDONLY | O_CREAT, S_IWUSR) == -EACCES);

	/* Invalid mode. */
	uassert(__nanvix_shm_open("cool-name", O_WRONLY | O_CREAT, modeinval) == -ENOTSUP);
}

/**
 * @brief Fault Injection Test: Invalid Create
 */
static void test_shm_create_inval(void)
{
	test_shm_create_inval1();
	test_shm_create_inval2();
}

/*============================================================================*
 * Fault Injection Test: Invalid Exclusive Create                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Exclusive Create
 */
static void test_shm_create_excl_inval(void)
{
	mode_t modeinval;
	char longname[SHM_LONG_NAME_LEN];

	modeinval = ~(S_IRUSR | S_IWUSR);

	for (size_t i = 0; i < SHM_LONG_NAME_LEN; i++)
		longname[i] = 'a';
	longname[SHM_LONG_NAME_LEN - 1] = '\0';

	/* Invalid name. */
	uassert(__nanvix_shm_open(NULL, O_WRONLY | O_CREAT | O_EXCL, S_IWUSR) == -EINVAL);
	uassert(__nanvix_shm_open("", O_WRONLY | O_CREAT | O_EXCL, S_IWUSR) == -EINVAL);
	uassert(__nanvix_shm_open(longname, O_WRONLY | O_CREAT | O_EXCL, S_IWUSR) == -ENAMETOOLONG);

	/* Invalid opening flag. */
	uassert(__nanvix_shm_open("cool-name", O_RDONLY | O_CREAT | O_EXCL, S_IWUSR) == -EACCES);

	/* Invalid mode. */
	uassert(__nanvix_shm_open("cool-name", O_WRONLY | O_CREAT | O_EXCL, modeinval) == -ENOTSUP);
}

/*============================================================================*
 * Fault Injection Test: Invalid Open                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Open
 */
static void test_shm_open_inval(void)
{
	int shmid;
	char longname[SHM_LONG_NAME_LEN];

	for (size_t i = 0; i < SHM_LONG_NAME_LEN; i++)
		longname[i] = 'a';
	longname[SHM_LONG_NAME_LEN - 1] = '\0';

	/* Invalid name. */
	uassert(__nanvix_shm_open(NULL, O_WRONLY, S_IWUSR) == -EINVAL);
	uassert(__nanvix_shm_open("", O_WRONLY, S_IWUSR) == -EINVAL);
	uassert(__nanvix_shm_open(longname, O_WRONLY, S_IWUSR) == -ENAMETOOLONG);

	/* Invalid opening flag. */

	uassert((shmid = __nanvix_shm_creat("cool-name", S_IWUSR)) >= 0);
	uassert(__nanvix_shm_open("cool-name", O_RDONLY | O_TRUNC, S_IWUSR) == -EACCES);
	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Unlink                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Unlink
 */
static void test_shm_unlink_inval(void)
{
	char longname[SHM_LONG_NAME_LEN];

	for (size_t i = 0; i < SHM_LONG_NAME_LEN; i++)
		longname[i] = 'a';
	longname[SHM_LONG_NAME_LEN - 1] = '\0';

	/* Invalid name. */
	uassert(__nanvix_shm_unlink(NULL) == -EINVAL);
	uassert(__nanvix_shm_unlink("") == -EINVAL);
	uassert(__nanvix_shm_unlink(longname) == -ENAMETOOLONG);
}

/*============================================================================*
 * Fault Injection Test: Invalid Close                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Close
 */
static void test_shm_close_inval(void)
{
	/* Invalid shared memory region ID. */
	uassert(__nanvix_shm_close(-1) == -EINVAL);
	uassert(__nanvix_shm_close(NANVIX_SHM_OPEN_MAX) == -ENOENT);
	uassert(__nanvix_shm_close(NANVIX_SHM_MAX) == -EINVAL);
}

/*============================================================================*
 * Fault Injection Test: Invalid Truncate                                     *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Truncate
 */
static void test_shm_ftruncate_inval(void)
{
	int shmid;

	/* Invalid shared memory region ID. */
	uassert(__nanvix_shm_ftruncate(-1, 0) == -EINVAL);
	uassert(__nanvix_shm_ftruncate(NANVIX_SHM_OPEN_MAX, 0) == -ENOENT);
	uassert(__nanvix_shm_ftruncate(NANVIX_SHM_MAX, 0) == -EINVAL);

	uassert((shmid = __nanvix_shm_open("cool-name", O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) >= 0);

		/* Invalid size. */
		uassert(__nanvix_shm_ftruncate(shmid, -1) == -EINVAL);
		uassert(__nanvix_shm_ftruncate(shmid, NANVIX_SHM_SIZE_MAX + 1) == -EFBIG);

	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Read                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void test_shm_read_inval(void)
{
	int shmid;

	/* Invalid shared memory region ID. */
	uassert(__nanvix_shm_read(-1, buffer, NANVIX_SHM_SIZE_MAX, 0) == -EINVAL);
	uassert(__nanvix_shm_read(NANVIX_SHM_OPEN_MAX, buffer, NANVIX_SHM_SIZE_MAX, 0) == -ENOENT);
	uassert(__nanvix_shm_read(NANVIX_SHM_MAX, buffer, NANVIX_SHM_SIZE_MAX, 0) == -EINVAL);

	/* Invalid shared memory region ID. */

	uassert((shmid = __nanvix_shm_open("cool-name", O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) >= 0);

		/* Invalid buffer. */
		uassert(__nanvix_shm_read(shmid, NULL, NANVIX_SHM_SIZE_MAX, 0) == -EINVAL);

		/* Invalid read size. */
		uassert(__nanvix_shm_read(shmid, buffer, 0, 0) == -EINVAL);
		uassert(__nanvix_shm_read(shmid, buffer, NANVIX_SHM_SIZE_MAX - 1, 0) == -EINVAL);
		uassert(__nanvix_shm_read(shmid, buffer, NANVIX_SHM_SIZE_MAX + 1, 0) == -EINVAL);

		/* Invalid offset. */
		uassert(__nanvix_shm_read(shmid, buffer, NANVIX_SHM_SIZE_MAX, -1) == -EINVAL);
		uassert(__nanvix_shm_read(shmid, buffer, NANVIX_SHM_SIZE_MAX, 1) == -EINVAL);

	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Invalid Write                                        *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void test_shm_write_inval(void)
{
	int shmid;

	/* Invalid shared memory region ID. */
	uassert(__nanvix_shm_write(-1, buffer, NANVIX_SHM_SIZE_MAX, 0) == -EINVAL);
	uassert(__nanvix_shm_write(NANVIX_SHM_OPEN_MAX, buffer, NANVIX_SHM_SIZE_MAX, 0) == -ENOENT);
	uassert(__nanvix_shm_write(NANVIX_SHM_MAX, buffer, NANVIX_SHM_SIZE_MAX, 0) == -EINVAL);

	/* Invalid shared memory region ID. */

	uassert((shmid = __nanvix_shm_open("cool-name", O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) >= 0);

		/* Invalid buffer. */
		uassert(__nanvix_shm_write(shmid, NULL, NANVIX_SHM_SIZE_MAX, 0) == -EINVAL);

		/* Invalid write size. */
		uassert(__nanvix_shm_write(shmid, buffer, 0, 0) == -EINVAL);
		uassert(__nanvix_shm_write(shmid, buffer, NANVIX_SHM_SIZE_MAX - 1, 0) == -EINVAL);
		uassert(__nanvix_shm_write(shmid, buffer, NANVIX_SHM_SIZE_MAX + 1, 0) == -EINVAL);

		/* Invalid offset. */
		uassert(__nanvix_shm_write(shmid, buffer, NANVIX_SHM_SIZE_MAX, -1) == -EINVAL);
		uassert(__nanvix_shm_write(shmid, buffer, NANVIX_SHM_SIZE_MAX, 1) == -EINVAL);

	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink("cool-name") == 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Unlink                                           *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Unlink
 */
static void test_shm_unlink_bad(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	/* Unlink non-existent shared memory region. */
	uassert(__nanvix_shm_unlink("cool-name") == -ENOENT);

	/* Unlink miss-spelled shared memory region. */
	uassert((shmid = __nanvix_shm_creat(shm_name, S_IWUSR)) >= 0);
	uassert(__nanvix_shm_unlink("cool-name") == -ENOENT);
	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Close                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Close
 */
static void test_shm_close_bad(void)
{
	int shmid;
	const char *shm_name = "cool-region";

	/* Close non-existent shared memory region. */
	uassert(__nanvix_shm_close(0) == -ENOENT);

	/* Close bad shared memory region. */
	uassert((shmid = __nanvix_shm_creat(shm_name, S_IWUSR)) >= 0);
	uassert(__nanvix_shm_close(1) == -ENOENT);
	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_shm_unlink(shm_name) == 0);
}

/*============================================================================*
 * Fault Injection Test: Bad Truncate                                         *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Truncate
 */
static void test_shm_ftruncate_bad(void)
{
	/* Bad shared memory region ID. */
	uassert(__nanvix_shm_ftruncate(0, 0) == -ENOENT);
}

/*============================================================================*
 * Fault Injection Test: Bad Read                                             *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Read
 */
static void test_shm_read_bad(void)
{
	/* Bad shared memory region ID. */
	uassert(__nanvix_shm_read(0, buffer, NANVIX_SHM_SIZE_MAX, 0) == -ENOENT);
}

/*============================================================================*
 * Fault Injection Test: Bad Write                                            *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Bad Write
 */
static void test_shm_write_bad(void)
{
	/* Bad shared memory region ID. */
	uassert(__nanvix_shm_write(0, buffer, NANVIX_SHM_SIZE_MAX, 0) == -ENOENT);
}

/*============================================================================*
 * Fault Injection Driver Table                                               *
 *============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_shm_fault[] = {
	{ test_shm_create_inval,      "invalid create     " },
	{ test_shm_create_excl_inval, "invalid excl create" },
	{ test_shm_open_inval,        "invalid open       " },
	{ test_shm_unlink_inval,      "invalid unlink     " },
	{ test_shm_close_inval,       "invalid close      " },
	{ test_shm_ftruncate_inval,   "invalid ftruncate  " },
	{ test_shm_read_inval,        "invalid read       " },
	{ test_shm_write_inval,       "invalid write      " },
	{ test_shm_unlink_bad,        "bad unlink         " },
	{ test_shm_close_bad,         "bad close          " },
	{ test_shm_ftruncate_bad,     "bad ftruncate      " },
	{ test_shm_read_bad,          "bad read           " },
	{ test_shm_write_bad,         "bad write          " },
	{ NULL,                        NULL                 },
};
