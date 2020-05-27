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
 * @brief Fault Injection Test: Bad Inode Alloc
 */
static void test_fault_bad_inode_alloc(void)
{
#ifdef TEST_BAD_FS
	struct filesystem fs_dummy;
#endif

	uassert(fprocess_launch(CONNECTION) == 0);

#ifdef TEST_BAD_FS
	inode_alloc(
		&fs_dummy,
		0,
		NANVIX_ROOT_UID,
		NANVIX_ROOT_GID
	);
	uassert(curr_proc->errcode == -EINVAL);
	curr_proc->errcode = 0;
#endif
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
 * @brief Fault Injection Test: Bad Inode Get
 */
static void test_fault_bad_inode_get(void)
{
	uassert(fprocess_launch(CONNECTION) == 0);

	inode_get(
		&fs_root,
		NANVIX_NR_INODES
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
 * @brief Fault Injection Test: Bad Inode Put
 */
static void test_fault_bad_inode_put(void)
{
	struct inode *ip;
	struct filesystem fs_dummy;

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
			&fs_dummy,
			ip
		);
		uassert(curr_proc->errcode == -EINVAL);
		curr_proc->errcode = 0;

	/* put */
	inode_put(
		&fs_root,
		ip
	);
	uassert(curr_proc->errcode == 0);

		inode_put(
			&fs_root,
			ip
		);
		uassert(curr_proc->errcode == -EINVAL);
		curr_proc->errcode = 0;
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
 * @brief Fault Injection Test: Bad Inode Write
 */
static void test_fault_bad_inode_write(void)
{
	struct inode *ip;
	struct filesystem fs_dummy;

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
			&fs_dummy,
			ip
		);
		uassert(curr_proc->errcode == -EINVAL);
		curr_proc->errcode = 0;

	/* put */
	inode_put(
		&fs_root,
		ip
	);
	uassert(curr_proc->errcode == 0);

		inode_write(
			&fs_root,
			ip
		);
		uassert(curr_proc->errcode == -EINVAL);
		curr_proc->errcode = 0;
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

/**
 * @brief Fault Injection Test: Bad Inode Touch
 */
static void test_fault_bad_inode_touch(void)
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

	inode_touch(ip);
	uassert(curr_proc->errcode == -EINVAL);
	curr_proc->errcode = 0;
}

/*============================================================================*
 * Stress Tets                                                                *
 *============================================================================*/

/**
 * @brief Stress Test: Inode Alloc/Free One Step
 */
static void test_stress_inode_alloc_free1(void)
{
	struct inode *ip;

	uassert(fprocess_launch(CONNECTION) == 0);

	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i++)
	{
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
}

/**
 * @brief Stress Test: Inode Alloc/Free Two Steps
 */
static void test_stress_inode_alloc_free2(void)
{
	struct inode *ip[NANVIX_INODES_TABLE_LENGTH];

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i++)
	{
		ip[i] = inode_alloc(
			&fs_root,
			0,
			NANVIX_ROOT_UID,
			NANVIX_ROOT_GID
		);
		uassert(curr_proc->errcode == 0);
	}

	/* put */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i++)
	{
		inode_put(
			&fs_root,
			ip[i]
		);
		uassert(curr_proc->errcode == 0);
	}
}

/**
 * @brief Stress Test: Inode Alloc/Free Three Steps
 */
static void test_stress_inode_alloc_free3(void)
{
	struct inode *ip[NANVIX_INODES_TABLE_LENGTH];

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH - 1; i += 2)
	{
		ip[i] = inode_alloc(
			&fs_root,
			0,
			NANVIX_ROOT_UID,
			NANVIX_ROOT_GID
		);
		uassert(curr_proc->errcode == 0);
		ip[i + 1] = inode_alloc(
			&fs_root,
			0,
			NANVIX_ROOT_UID,
			NANVIX_ROOT_GID
		);
		uassert(curr_proc->errcode == 0);
		inode_put(
			&fs_root,
			ip[i + 1]
		);
		uassert(curr_proc->errcode == 0);
	}

	/* put */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i += 2)
	{
		inode_put(
			&fs_root,
			ip[i]
		);
		uassert(curr_proc->errcode == 0);
	}
}

/**
 * @brief Stress Test: Inode Get/Put One Step
 */
static void test_stress_inode_get_put1(void)
{
	struct inode *ip1;
	struct inode *ip2;

	uassert(fprocess_launch(CONNECTION) == 0);

	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i++)
	{
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
}

/**
 * @brief Stress Test: Inode Get/Put Two Steps
 */
static void test_stress_inode_get_put2(void)
{
	struct inode *ip[NANVIX_INODES_TABLE_LENGTH];

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i++)
	{
		ip[i] = inode_alloc(
			&fs_root,
			0,
			NANVIX_ROOT_UID,
			NANVIX_ROOT_GID
		);
		uassert(curr_proc->errcode == 0);

			inode_get(
				&fs_root,
				inode_get_num(ip[i])
			);
			uassert(curr_proc->errcode == 0);
	}

	/* put */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i++)
	{
			inode_put(
				&fs_root,
				ip[i]
			);
			uassert(curr_proc->errcode == 0);

		inode_put(
			&fs_root,
			ip[i]
		);
		uassert(curr_proc->errcode == 0);
	}
}

/**
 * @brief Stress Test: Inode Get/Put Three Steps
 */
static void test_stress_inode_get_put3(void)
{
	struct inode *ip[NANVIX_INODES_TABLE_LENGTH];

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH - 1; i += 2)
	{
		ip[i] = inode_alloc(
			&fs_root,
			0,
			NANVIX_ROOT_UID,
			NANVIX_ROOT_GID
		);
		uassert(curr_proc->errcode == 0);
		ip[i + 1] = inode_alloc(
			&fs_root,
			0,
			NANVIX_ROOT_UID,
			NANVIX_ROOT_GID
		);
		uassert(curr_proc->errcode == 0);
		inode_put(
			&fs_root,
			ip[i + 1]
		);
		uassert(curr_proc->errcode == 0);

			inode_get(
				&fs_root,
				inode_get_num(ip[i])
			);
			uassert(curr_proc->errcode == 0);
	}

	/* put */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i += 2)
	{
			inode_put(
				&fs_root,
				ip[i]
			);
			uassert(curr_proc->errcode == 0);

		inode_put(
			&fs_root,
			ip[i]
		);
		uassert(curr_proc->errcode == 0);
	}
}

/**
 * @brief Stress Test: Inode Touch/Write One Step
 */
static void test_stress_inode_touch_write1(void)
{
	struct inode *ip1;
	struct inode *ip2;

	uassert(fprocess_launch(CONNECTION) == 0);

	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i++)
	{
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

				/* touch */
				inode_touch(ip2);
				uassert(curr_proc->errcode == 0);

				/* write */
				inode_write(
					&fs_root,
					ip2
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
}

/**
 * @brief Stress Test: Inode Touch/Write Two Steps
 */
static void test_stress_inode_touch_write2(void)
{
	struct inode *ip2;
	struct inode *ip[NANVIX_INODES_TABLE_LENGTH];

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i++)
	{
		ip[i] = inode_alloc(
			&fs_root,
			0,
			NANVIX_ROOT_UID,
			NANVIX_ROOT_GID
		);
		uassert(curr_proc->errcode == 0);

			ip2 = inode_get(
				&fs_root,
				inode_get_num(ip[i])
			);
			uassert(curr_proc->errcode == 0);

				/* touch */
				inode_touch(ip2);
				uassert(curr_proc->errcode == 0);
	}

	/* put */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i++)
	{
				/* write */
				inode_write(
					&fs_root,
					ip[i]
				);
				uassert(curr_proc->errcode == 0);

			inode_put(
				&fs_root,
				ip[i]
			);
			uassert(curr_proc->errcode == 0);

		inode_put(
			&fs_root,
			ip[i]
		);
		uassert(curr_proc->errcode == 0);
	}
}

/**
 * @brief Stress Test: Inode Touch/Write Three Steps
 */
static void test_stress_inode_touch_write3(void)
{
	struct inode *ip2;
	struct inode *ip[NANVIX_INODES_TABLE_LENGTH];

	uassert(fprocess_launch(CONNECTION) == 0);

	/* alloc */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH - 1; i += 2)
	{
		ip[i] = inode_alloc(
			&fs_root,
			0,
			NANVIX_ROOT_UID,
			NANVIX_ROOT_GID
		);
		uassert(curr_proc->errcode == 0);
		ip[i + 1] = inode_alloc(
			&fs_root,
			0,
			NANVIX_ROOT_UID,
			NANVIX_ROOT_GID
		);
		uassert(curr_proc->errcode == 0);
		inode_put(
			&fs_root,
			ip[i + 1]
		);
		uassert(curr_proc->errcode == 0);

			ip2 = inode_get(
				&fs_root,
				inode_get_num(ip[i])
			);
			uassert(curr_proc->errcode == 0);

				/* touch */
				inode_touch(ip2);
				uassert(curr_proc->errcode == 0);
	}

	/* put */
	for (int i = 2; i < NANVIX_INODES_TABLE_LENGTH; i += 2)
	{
				/* write */
				inode_write(
					&fs_root,
					ip[i]
				);
				uassert(curr_proc->errcode == 0);

			inode_put(
				&fs_root,
				ip[i]
			);
			uassert(curr_proc->errcode == 0);

		inode_put(
			&fs_root,
			ip[i]
		);
		uassert(curr_proc->errcode == 0);
	}
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
	{ test_api_inode_alloc_free,        "[inode][api] alloc free                 " },
	{ test_api_inode_get_put,           "[inode][api] get put                    " },
	{ test_api_inode_write,             "[inode][api] write                      " },
	{ test_api_inode_touch,             "[inode][api] touch                      " },
	{ test_fault_invalid_inode_get_num, "[inode][fault] invalid get number       " },
	{ test_fault_invalid_inode_alloc,   "[inode][fault] invalid alloc            " },
	{ test_fault_invalid_inode_get,     "[inode][fault] invalid get              " },
	{ test_fault_invalid_inode_put,     "[inode][fault] invalid put              " },
	{ test_fault_invalid_inode_write,   "[inode][fault] invalid write            " },
	{ test_fault_invalid_inode_touch,   "[inode][fault] invalid touch            " },
	{ test_fault_bad_inode_alloc,       "[inode][fault] bad alloc                " },
	{ test_fault_bad_inode_get,         "[inode][fault] bad get                  " },
	{ test_fault_bad_inode_put,         "[inode][fault] bad put                  " },
	{ test_fault_bad_inode_write,       "[inode][fault] bad write                " },
	{ test_fault_bad_inode_touch,       "[inode][fault] bad touch                " },
	{ test_stress_inode_alloc_free1,    "[inode][stress] alloc free one step     " },
	{ test_stress_inode_alloc_free2,    "[inode][stress] alloc free two steps    " },
	{ test_stress_inode_alloc_free3,    "[inode][stress] alloc free three steps  " },
	{ test_stress_inode_get_put1,       "[inode][stress] get put one step        " },
	{ test_stress_inode_get_put2,       "[inode][stress] get put two steps       " },
	{ test_stress_inode_get_put3,       "[inode][stress] get put three steps     " },
	{ test_stress_inode_touch_write1,   "[inode][stress] touch write one step    " },
	{ test_stress_inode_touch_write2,   "[inode][stress] touch write two steps   " },
	{ test_stress_inode_touch_write3,   "[inode][stress] touch write three steps " },
	{ NULL,                              NULL                                      },
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
