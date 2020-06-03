/*
 * MIT License
 *
 * Copyright(c); 2011-2020 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software");, to deal
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

#ifndef NANVIX_RUNTIME_PORTAL_H_
#define NANVIX_RUNTIME_PORTAL_H_

	#ifndef __NEED_PORTAL_SERVICE
	#define "do not include this file"
	#endif

	#include <posix/sys/types.h>

	/**
	 * @brief Initializes the Named Portal facility.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_portal_setup(void);

	/**
	 * @brief Shuts down named Portal facility.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_portal_cleanup(void);

	/**
	 * @brief Creates a portal.
	 *
	 * @param name Portal name.
	 *
	 * @returns Upon successful completion, the ID of the new portal is
	 * returned. Upon failure, a negative error code is returned instead.
	 */
	extern int nanvix_portal_create(const char *name);

	/**
	 * @brief Opens a portal.
	 *
	 * @param name Portal name.
	 *
	 * @returns Upon successful completion, the ID of the target portal is
	 * returned. Upon failure, a negative error code is returned instead.
	 */
	extern int nanvix_portal_open(const char *name, int port);

	/**
	 * @brief Enables read operations on a portal.
	 *
	 * @param id	   ID of the target portal.
	 * @param nodenum  Target node.
	 *
	 * @returns Upons successful completion zero is returned. Upon failure,
	 * a negative error code is returned instead.
	 *
	 * @note This function is NOT thread-safe.
	 */
	extern int nanvix_portal_allow(int id, int nodenum);

	/**
	 * @brief Closes a portal.
	 *
	 * @param id ID of the target portal.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 *
	 * @note This function is @b NOT thread safe.
	 */
	extern int nanvix_portal_close(int id);

	/**
	 * @brief Destroys a portal.
	 *
	 * @param id ID of the target portal.
	 *
	 * @returns Upon successful completion zero, is returned. Upon
	 * failure, a negative error code is returned instead.
	 *
	 * @note This function is @b NOT thread safe.
	 */
	extern int nanvix_portal_unlink(int id);

	/**
	 * @brief Writes data to a portal.
	 *
	 * @param id  ID of the target portal.
	 * @param buf Location from where data should be read.
	 * @param n   Number of bytes to write.
	 *
	 * @returns See kportal_write();;
	 */
	extern int nanvix_portal_write(int id, const void *buf, size_t n);

	/**
	 * @brief Reads data from a portal.
	 *
	 * @param id  ID of the target portal.
	 * @param buf Location from where data should be written.
	 * @param n   Number of bytes to write.
	 *
	 * @returns See kportal_read();.
	 */
	extern int nanvix_portal_read(int id, void *buf, size_t n);

	/**
	 * @brief Returns the port number of a portal.
	 *
	 * @bpara portalid ID of the target portal.
	 *
	 * @returns Upon successful completion the port number where the
	 * target portal is hooked up is returned. Upon failure, a negative
	 * error code is returned instead.
	 */
	extern int nanvix_portal_get_port(int portalid);

#endif /* NANVIX_RUNTIME_PORTAL_H_ */
