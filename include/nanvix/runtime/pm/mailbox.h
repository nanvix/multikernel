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

#ifndef NANVIX_RUNTIME_MAILBOX_H_
#define NANVIX_RUNTIME_MAILBOX_H_

	#ifndef __NEED_MAILBOX_SERVICE
	#define "do not include this file"
	#endif

	#include <posix/sys/types.h>

	/**
	 * @brief Initializes the Named Mailbox facility.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_mailbox_setup(void);

	/**
	 * @brief Shuts down Named Mailbox facility.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int __nanvix_mailbox_cleanup(void);

	/**
	 * @brief Get named input mailbox.
	 */
	extern int nanvix_mailbox_get(void);

	/**
	 * @brief Creates a mailbox.
	 *
	 * @param name Mailbox name.
	 *
	 * @returns Upon successful completion, the ID of the new mailbox is
	 * returned. Upon failure, a negative error code is returned instead.
	 */
	extern int nanvix_mailbox_create(const char *name);

	/**
	 * @brief Creates a mailbox specifying the input port.
	 *
	 * @param name Mailbox name.
	 * @param port Local port.
	 *
	 * @returns Upon successful completion, the ID of the new mailbox is
	 * returned. Upon failure, a negative error code is returned instead.
	 *
	 * @note This function should be called only for creating mailboxes with
	 * ports specified in the special range that is not related with the
	 * standard ones.
	 */
	extern int nanvix_mailbox_create2(const char *name, int port);

	/**
	 * @brief Opens a mailbox.
	 *
	 * @param name Mailbox name.
	 * @param port Remote port.
	 *
	 * @returns Upon successful completion, the ID of the target mailbox is
	 * returned. Upon failure, a negative error code is returned instead.
	 */
	extern int nanvix_mailbox_open(const char *name, int port);

	/**
	 * @brief Reads data from a mailbox.
	 *
	 * @param mbxid ID of the target mailbox.
	 * @param buf   Location from where data should be written.
	 * @param n     Number of bytes to write.
	 *
	 * @returns Upon successful completion zero is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	extern int nanvix_mailbox_read(int mbxid, void *buf, size_t n);

	/**
	 * @brief Specifies the mailbox remote for a single read.
	 *
	 * @param mbxid  ID of the target mailbox.
	 * @param remote Remote nodenum.
	 * @param port   Remote port number.
	 *
	 * @returns Upon successful completion zero is returned. Upon failure,
	 * a negative error code is returned instead.
	 */
	extern int nanvix_mailbox_set_remote(int mbxid, int remote, int port);

	/**
	 * @brief Writes data to a mailbox.
	 *
	 * @param mbxid ID of the target mailbox.
	 * @param buf   Location from where data should be read.
	 * @param n     Number of bytes to write.
	 *
	 * @returns Upon successful completion zero is returned. Upon failure, a
	 * negative error code is returned instead.
	 */
	extern int nanvix_mailbox_write(int mbxid, const void *buf, size_t n);

	/**
	 * @brief Closes a mailbox.
	 *
	 * @param mbxid ID of the target mailbox.
	 *
	 * @returns Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 *
	 * @note This function is @b NOT thread safe.
	 */
	int nanvix_mailbox_close(int mbxid);

	/**
	 * @brief Destroys a mailbox.
	 *
	 * @param mbxid ID of the target mailbox.
	 *
	 * @returns Upon successful completion zero, is returned. Upon
	 * failure, a negative error code is returned instead.
	 *
	 * @note This function is @b NOT thread safe.
	 */
	int nanvix_mailbox_unlink(int mbxid);

	/**
	 * @brief Returns the port number of a mailbox.
	 *
	 * @bpara mbxid ID of the target mailbox.
	 *
	 * @returns Upon successful completion the port number where the
	 * target mailbox is hooked up is returned. Upon failure, a negative
	 * error code is returned instead.
	 */
	extern int nanvix_mailbox_get_port(int mbxid);

#endif /* NANVIX_RUNTIME_MAILBOX_H_ */
