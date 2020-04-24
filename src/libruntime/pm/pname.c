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
#define __NEED_LIMITS_PM

#include <nanvix/limits/pm.h>
#include <nanvix/runtime/pm.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/**
 * @brief Name of the running process.
 */
static char _pname[NANVIX_PROC_NAME_MAX] = "";

/*============================================================================*
 * get_pname ()                                                               *
 *============================================================================*/

/**
 * The nanvix_getpname() function returns the name of the calling
 * process.
 */
const char *nanvix_getpname(void)
{
	return (_pname);
}

/*============================================================================*
 * get_pname ()                                                               *
 *============================================================================*/

/**
 * The nanvix_setpname() function sets the name of the calling process
 * to @pname.
 */
int nanvix_setpname(const char *pname)
{
	int ret;
	int nodenum;

	/* Invalid name. */
	if (ustrlen(pname) >= NANVIX_PROC_NAME_MAX)
		return (-EINVAL);

	if (ustrcmp(_pname, ""))
		return (-EBUSY);

	nodenum = knode_get_num();

	/* Link process name. */
	if ((ret = name_link(nodenum, pname)) < 0)
		return (ret);

	return (0);
}
