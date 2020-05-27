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
#include "nanvix/config.h"
#define __VFS_SERVER

#include <nanvix/servers/vfs.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/fcntl.h>

/**
 * @brief Connection Used in Tests
 */
#define CONNECTION 0

/*============================================================================*
 * API Tests                                                                  *
 *============================================================================*/

/**
 * @brief API Test: Inode Alloc/Free
 */
static void test_api_inode_alloc_free(void)
{
	struct inode *ip;

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	ip = inode_alloc(
		&fs_root,
		0,
		NANVIX_ROOT_UID,
		NANVIX_ROOT_GID
	);
	uassert(curr_proc->errcode == 0);

	/* put */
	inode_put(
		&fs_root,
		ip
	);
	uassert(curr_proc->errcode == 0);
}

/**
 * @brief API Test: Inode Get/Put
 */
static void test_api_inode_get_put(void)
{
	struct inode *ip1;
	struct inode *ip2;

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	ip1 = inode_alloc(
		&fs_root,
		0,
		NANVIX_ROOT_UID,
		NANVIX_ROOT_GID
	);
	uassert(curr_proc->errcode == 0);

		/* get */
		ip2 = inode_get(
			&fs_root,
			inode_get_num(ip1)
		);
		uassert(curr_proc->errcode == 0);

		/* put */
		inode_put(
			&fs_root,
			ip2
		);
		uassert(curr_proc->errcode == 0);

	/* put */
	inode_put(
		&fs_root,
		ip1
	);
	uassert(curr_proc->errcode == 0);
}

/**
 * @brief API Test: Inode Write
 */
static void test_api_inode_write(void)
{
	struct inode *ip;

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	ip = inode_alloc(
		&fs_root,
		0,
		NANVIX_ROOT_UID,
		NANVIX_ROOT_GID
	);
	uassert(curr_proc->errcode == 0);

		/* write */
		inode_write(
			&fs_root,
			ip
		);
		uassert(curr_proc->errcode == 0);

	/* put */
	inode_put(
		&fs_root,
		ip
	);
	uassert(curr_proc->errcode == 0);
}

/**
 * @brief API Test: Inode Touch
 */
static void test_api_inode_touch(void)
{
	struct inode *ip;

	/* alloc */
	ip = inode_alloc(
		&fs_root,
		0,
		NANVIX_ROOT_UID,
		NANVIX_ROOT_GID
	);
	uassert(curr_proc->errcode == 0);

		/* touch */
		inode_touch(ip);
		uassert(curr_proc->errcode == 0);

		/* write */
		inode_write(
			&fs_root,
			ip
		);
		uassert(curr_proc->errcode == 0);

	/* put */
	inode_put(
		&fs_root,
		ip
	);
	uassert(curr_proc->errcode == 0);
}

/*============================================================================*
 * Fault Injection Tets                                                       *
 *============================================================================*/

/**
 * @brief Fault Injection Test: Invalid Inode Get Number
 */
static void test_fault_invalid_inode_get_num(void)
{
	uassert(fprocess_launch(CONNECTION) == 0);

	inode_get_num(NULL);
	uassert(curr_proc->errcode == -EINVAL);
	curr_proc->errcode = 0;
}

/**
 * @brief Fault Injection Test: Invalid Inode Alloc
 */
static void test_fault_invalid_inode_alloc(void)
{
	uassert(fprocess_launch(CONNECTION) == 0);

	inode_alloc(
		NULL,
		0,
		NANVIX_ROOT_UID,
		NANVIX_ROOT_GID
	);
	uassert(curr_proc->errcode == -EINVAL);
	curr_proc->errcode = 0;
}

/**
 * @brief Fault Injection Test: Invalid Inode Get
 */
static void test_fault_invalid_inode_get(void)
{
	uassert(fprocess_launch(CONNECTION) == 0);

	inode_get(
		NULL,
		0
	);
	uassert(curr_proc->errcode == -EINVAL);
	curr_proc->errcode = 0;
}

/**
 * @brief Fault Injection Test: Invalid Inode Put
 */
static void test_fault_invalid_inode_put(void)
{
	struct inode *ip;

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	ip = inode_alloc(
		&fs_root,
		0,
		NANVIX_ROOT_UID,
		NANVIX_ROOT_GID
	);
	uassert(curr_proc->errcode == 0);

		inode_put(
			NULL,
			ip
		);
		uassert(curr_proc->errcode == -EINVAL);
		curr_proc->errcode = 0;

		inode_put(
			&fs_root,
			NULL
		);
		uassert(curr_proc->errcode == -EINVAL);
		curr_proc->errcode = 0;

	/* put */
	inode_put(
		&fs_root,
		ip
	);
	uassert(curr_proc->errcode == 0);
}

/**
 * @brief Fault Injection Test: Invalid Inode Write
 */
static void test_fault_invalid_inode_write(void)
{
	struct inode *ip;

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	ip = inode_alloc(
		&fs_root,
		0,
		NANVIX_ROOT_UID,
		NANVIX_ROOT_GID
	);
	uassert(curr_proc->errcode == 0);

		inode_write(
			NULL,
			ip
		);
		uassert(curr_proc->errcode == -EINVAL);
		curr_proc->errcode = 0;

		inode_write(
			&fs_root,
			NULL
		);
		uassert(curr_proc->errcode == -EINVAL);
		curr_proc->errcode = 0;

	/* put */
	inode_put(
		&fs_root,
		ip
	);
	uassert(curr_proc->errcode == 0);
}

/**
 * @brief Fault Injection Test: Invalid Inode Touch
 */
static void test_fault_invalid_inode_touch(void)
{
	struct inode *ip;

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	ip = inode_alloc(
		&fs_root,
		0,
		NANVIX_ROOT_UID,
		NANVIX_ROOT_GID
	);
	uassert(curr_proc->errcode == 0);

		inode_touch(NULL);
		uassert(curr_proc->errcode == -EINVAL);
		curr_proc->errcode = 0;

	/* put */
	inode_put(
		&fs_root,
		ip
	);
	uassert(curr_proc->errcode == 0);
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
} vfs_tests[] = {
	{ test_api_inode_alloc_free,        "[inode][api] alloc free         " },
	{ test_api_inode_get_put,           "[inode][api] get put            " },
	{ test_api_inode_write,             "[inode][api] write              " },
	{ test_api_inode_touch,             "[inode][api] touch              " },
	{ test_fault_invalid_inode_get_num, "[inode][api] invalid get number " },
	{ test_fault_invalid_inode_alloc,   "[inode][api] invalid alloc      " },
	{ test_fault_invalid_inode_get,     "[inode][api] invalid get        " },
	{ test_fault_invalid_inode_put,     "[inode][api] invalid put        " },
	{ test_fault_invalid_inode_write,   "[inode][api] invalid write      " },
	{ test_fault_invalid_inode_touch,   "[inode][api] invalid touch      " },
	{ NULL,                              NULL                              },
};

/**
 * @brief Runs regression tests on Virtual File System
 */
void test_inode(void)
{
	for (int i = 0; vfs_tests[i].func != NULL; i++)
	{
		vfs_tests[i].func();

		uprintf("[nanvix][vfs]%s passed", vfs_tests[i].name);
	}
}
