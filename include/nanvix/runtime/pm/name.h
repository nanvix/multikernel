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

#ifndef NANVIX_RUNTIME_PM_NAME_H_
#define NANVIX_RUNTIME_PM_NAME_H_

	#ifndef __NEED_NAME_SERVICE
	#define "do not include this file"
	#endif

#ifdef __NAME_SERVICE

	#include <nanvix/servers/name.h>

#endif /* __NAME_SERVICE */

	/**
	 * @brief Initializes the Name Service client.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __name_setup(void);

	/**
	 * @brief Shuts down the Name Service client.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __name_cleanup(void);

	/**
	 * @brief link a process name.
	 *
	 * @param nodenum NoC node ID of the process to link.
	 * @param name   Name of the process to link.
	 *
	 * @returns Upon successful completion 0 is returned.
	 * Upon failure, a negative error code is returned instead.
	 */
	extern int name_link(int nodenum, const char *name);

	/**
	 * @brief Converts a name into a NoC node ID.
	 *
	 * @param name Target name.
	 *
	 * @returns Upon successful completion the NoC node ID whose name is @p
	 * name is returned. Upon failure, a negative error code is returned
	 * instead.
	 */
	extern int name_lookup(const char *name);

	/**
	 * @brief Unlink a process name.
	 *
	 * @param name Name of the process to unlink.
	 *
	 * @returns Upon successful completion zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int name_unlink(const char *name);

	/**
	 * @brief Updates the timestamp of a process.
	 *
	 * @returns Upons successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int name_heartbeat(void);

	/**
	 * @brief Shutdowns the name server.
	 *
	 * @returns Upon successful completion 0 is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int name_shutdown(void);

#endif /* NANVIX_RUNTIME_PM_NAME_H_ */
