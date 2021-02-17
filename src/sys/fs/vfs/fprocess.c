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
#define __VFS_SERVER

#include <nanvix/servers/vfs.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

/**
 * @brief Table of Processes
 */
static struct fprocess processes[NANVIX_CONNECTIONS_MAX];

/**
 * Current Process
 */
struct fprocess *curr_proc = NULL;

/*============================================================================*
 * fprocess_launch()                                                          *
 *============================================================================*/

/**
 * The fprocess_launc() function starts the file system process. This
 * process is hooked up to the connection @p connection.
 */
int fprocess_launch(int connection)
{
	/* Invalid connection. */
	if (!WITHIN(connection, 0, NANVIX_CONNECTIONS_MAX))
		return (-EINVAL);

	curr_proc = &processes[connection];

	/* Initialize process. */
	curr_proc->errcode = 0;

	return (0);
}

/*============================================================================*
 * fprocess_init()                                                            *
 *============================================================================*/

/**
 * The fprocess_init() function initializes the table of file system
 * processes.
 */
void fprocess_init(void)
{
	/* Initialize table of file system processes. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
	{
		processes[i].errcode = 0;
		processes[i].umask = 002;
		processes[i].pwd = fs_root.root;
		processes[i].root = fs_root.root;
		for (int j = 0; j < NANVIX_OPEN_MAX; j++)
			processes[i].ofiles[j] = NULL;
	}
}
