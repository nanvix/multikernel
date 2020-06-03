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

#ifndef NANVIX_RUNTIME_PM_H_
#define NANVIX_RUNTIME_PM_H_

	/* Must come first. */
	#define __NEED_NAME_SERVICE
	#define __NEED_MAILBOX_SERVICE
	#define __NEED_PORTAL_SERVICE

	#include <nanvix/runtime/stdikc.h>
	#include <nanvix/runtime/pm/name.h>
	#include <nanvix/runtime/pm/mailbox.h>
	#include <nanvix/runtime/pm/portal.h>

	/**
	 * @brief Gets the name of the process.
	 *
	 * @returns The name of the calling process.
	 */
	extern const char *nanvix_getpname(void);

	/**
	 * @brief Sets the name of the process.
	 *
	 * @param pname Process name.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_setpname(const char *pname);

#endif /* NANVIX_RUNTIME_PM_H_ */
