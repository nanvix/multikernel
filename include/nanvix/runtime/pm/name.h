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
	extern int __nanvix_name_setup(void);

	/**
	 * @brief Shuts down the Name Service client.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_name_cleanup(void);

	/**
	 * @brief link a process name.
	 *
	 * @param nodenum NoC node ID of the process to link.
	 * @param name   Name of the process to link.
	 *
	 * @returns Upon successful completion 0 is returned.
	 * Upon failure, a negative error code is returned instead.
	 */
	extern int nanvix_name_link(int nodenum, const char *name);

	/**
	 * @brief Converts a name into a NoC node ID.
	 *
	 * @param name Target name.
	 *
	 * @returns Upon successful completion the NoC node ID whose name is @p
	 * name is returned. Upon failure, a negative error code is returned
	 * instead.
	 */
	extern int nanvix_name_lookup(const char *name);

	/**
	 * @brief Unlink a process name.
	 *
	 * @param name Name of the process to unlink.
	 *
	 * @returns Upon successful completion zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_name_unlink(const char *name);

	/**
	 * @brief Links a process name and registers it in the local resolution table.
	 *
	 * @param nodenum NoC node ID of the process to link.
	 * @param name   Name of the process to link.
	 *
	 * @returns Upon successful completion 0 is returned.
	 * Upon failure, a negative error code is returned instead.
	 *
	 * @note This function also calls nanvix_name_link to assure that @p name
	 * exists in the remote name server and is visible for the other nodes.
	 * The name_link routine is called assuming that @p name corresponds to a
	 * locally present process, passing the local node number as one of the
	 * arguments.
	 */
	extern int nanvix_name_register(const char *name, int port_nr);

	/**
	 * @brief Converts a name into its logical address inside the system.
	 *
	 * @param name Target name.
	 * @param port Returned port holder.
	 *
	 * @returns Upon successful completion the NoC node ID whose name is @p
	 * name is returned with @p port containing the port number of the default
	 * inbox associated. Upon failure, a negative error code is returned
	 * instead.
	 */
	extern int nanvix_name_address_lookup(const char *name, int *port);

	/**
	 * @brief Unlink a process name.
	 *
	 * @param name Name of the process to unlink.
	 *
	 * @returns Upon successful completion zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 *
	 * @note This function also calls nanvix_name_unlink to remove @p name
	 * from the translation table of the remote server.
	 */
	extern int nanvix_name_unregister(const char *name);

	/**
	 * @brief Updates the timestamp of a process.
	 *
	 * @returns Upons successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_name_heartbeat(void);

	/**
	 * @brief Shutdowns the name server.
	 *
	 * @returns Upon successful completion 0 is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_name_shutdown(void);

#endif /* NANVIX_RUNTIME_PM_NAME_H_ */
