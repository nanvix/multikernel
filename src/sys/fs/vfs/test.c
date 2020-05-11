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

#include <nanvix/config.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include "ramdisk.h"

/**
 * @brief Bufer for Read/Write Tests
 */
static char data[512];

/*============================================================================*
 * RAM Disk Tests                                                             *
 *============================================================================*/

/**
 * @brief API Test: Read/Write to RAM Disk
 */
static void ramdisk_api_read_write(void)
{
	umemset(data, 1, sizeof(data));

	for (unsigned minor = 0; minor < NANVIX_FS_NR_RAMDISKS; minor++)
	{
		ramdisk_write(minor, data, sizeof(data), 0);
		ramdisk_read(minor, data, sizeof(data), 0);

		/* Checksum. */
		for (size_t i = 0; i < sizeof(data); i++)
			uassert(data[i] == 1);
	}
}

/**
 * @brief Fault Injection Test: Invalid Read
 */
static void ramdisk_fault_read_inval(void)
{
	uassert(ramdisk_read(-1, data, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_read(NANVIX_FS_NR_RAMDISKS, data, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_read(0, NULL, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_read(0, data, -1, 0) == -EINVAL);
	uassert(ramdisk_read(0, data, NANVIX_FS_RAMDISK_SIZE + 1, 0) == -EINVAL);
	uassert(ramdisk_read(0, data, NANVIX_FS_RAMDISK_SIZE, 1) == -EINVAL);
	uassert(ramdisk_read(0, data, NANVIX_FS_RAMDISK_SIZE, -1) == -EINVAL);
}

/**
 * @brief Fault Injection Test: Invalid Write
 */
static void ramdisk_fault_write_inval(void)
{
	uassert(ramdisk_write(-1, data, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_write(NANVIX_FS_NR_RAMDISKS, data, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_write(0, NULL, sizeof(data), 0) == -EINVAL);
	uassert(ramdisk_write(0, data, -1, 0) == -EINVAL);
	uassert(ramdisk_write(0, data, NANVIX_FS_RAMDISK_SIZE + 1, 0) == -EINVAL);
	uassert(ramdisk_write(0, data, NANVIX_FS_RAMDISK_SIZE, 1) == -EINVAL);
	uassert(ramdisk_write(0, data, NANVIX_FS_RAMDISK_SIZE, -1) == -EINVAL);
}

/**
 * @brief Stress Test: Read/Write to RAM Disk
 */
static void ramdisk_stress_read_write(void)
{
	umemset(data, 1, sizeof(data));

	for (unsigned minor = 0; minor < NANVIX_FS_NR_RAMDISKS; minor++)
	{
		for (off_t off = 0; off < NANVIX_FS_RAMDISK_SIZE; off += sizeof(data))
		{
			ramdisk_write(minor, data, sizeof(data), 0);
			ramdisk_read(minor, data, sizeof(data), 0);

		/* Checksum. */
		for (size_t i = 0; i < sizeof(data); i++)
			uassert(data[i] == 1);
		}
	}
}

/**
 * @brief RAM Disk Tests
 */
static struct
{
	void (*func)(void); /**< Test Function */
	const char *name;   /**< Test Name     */
} ramdisk_tests[] = {
	{ ramdisk_api_read_write,    "[ramdisk][api]    read/write"    },
	{ ramdisk_fault_read_inval,  "[ramdisk][fault]  invalid read"  },
	{ ramdisk_fault_write_inval, "[ramdisk][fault]  invalid write" },
	{ ramdisk_stress_read_write, "[ramdisk][stress] read/write"    },
	{ NULL,                       NULL                             },
};

/**
 * @brief Runs regression tests on RAM Disk.
 */
static void ramdisk_test(void)
{
	for (int i = 0; ramdisk_tests[i].func != NULL; i++)
	{
		ramdisk_tests[i].func();

		uprintf("[nanvix][vfs]%s passed", ramdisk_tests[i].name);
	}
}

/*============================================================================*
 * RAM Disk Tests                                                             *
 *============================================================================*/

/**
 * @brief Runs regression tests on VFS.
 */
void vfs_test(void)
{
	ramdisk_test();
}
