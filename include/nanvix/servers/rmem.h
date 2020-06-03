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

#ifndef NANVIX_SERVERS_RMEM_H_
#define NANVIX_SERVERS_RMEM_H_

#ifdef __NEED_RMEM_SERVER

	#include <nanvix/types/mm/rmem.h>
	#include <nanvix/servers/message.h>
	#include <nanvix/config.h>
	#include <posix/stdint.h>
	#include <posix/stddef.h>

#endif /* __NEED_RMEM_SERVER */

	/**
	 * @brief Null remote address.
	 */
	#define RMEM_NULL 0

	/**
	 * @brief Remote memory shift.
	 */
	#define RMEM_BLOCK_SHIFT 12

	/**
	 * @brief Remote memory block size (in bytes).
	 */
	#define RMEM_BLOCK_SIZE (1 << RMEM_BLOCK_SHIFT)

	/**
	 * @brief Remote memory size (in bytes).
	 */
	#define RMEM_SIZE (32*1024*1024)

	/**
	 * @brief Number of remote memory blocks.
	 */
	#define RMEM_NUM_BLOCKS (RMEM_SIZE/RMEM_BLOCK_SIZE)

	/**
	 * @brief Size of payload for RMem messages.
	 */
	#ifdef __RMEM_USES_MAILBOX
	#define RMEM_PAYLOAD_SIZE 64
	#endif

	/**
	 * @name Shifts for remote addresses.
	 */
	/**@{*/
	#define RMEM_BLOCK_NUM_SHIFT    0ULL
	#define RMEM_BLOCK_SERVER_SHIFT 24ULL
	/**@}*/

	/**
	 * @name Masks for remote addresses.
	 */
	/**@{*/
	#define RMEM_BLOCK_NUM_MASK    (0xffffff << RMEM_BLOCK_NUM_SHIFT)    /**< Block Number  */
	#define RMEM_BLOCK_SERVER_MASK (    0xff << RMEM_BLOCK_SERVER_SHIFT) /**< Server Number */
	/**@}*/

	/**
	 * @brief Gets block number.
	 *
	 * The @p RMEM_BLOCK_NUM macro returns the number of the remote
	 * memory block encoded in a remote memory address.
	 *
	 * @param x Target remote address.
	 */
	#define RMEM_BLOCK_NUM(x) \
		(((x) & RMEM_BLOCK_NUM_MASK) >> RMEM_BLOCK_NUM_SHIFT)

	/**
	 * @brief Gets server number.
	 *
	 * The @p RMEM_BLOCK_SERVER macro returns the number of the remote
	 * memory server encoded in a remote memory address.
	 *
	 * @param x Target remote address.
	 */
	#define RMEM_BLOCK_SERVER(x) \
		(((x) & RMEM_BLOCK_SERVER_MASK) >> RMEM_BLOCK_SERVER_SHIFT)


	/**
	 * @brief Builds a remote memory address.
	 */
	#define RMEM_BLOCK(server, num) ( \
		(((word_t)(server) << RMEM_BLOCK_SERVER_SHIFT) & RMEM_BLOCK_SERVER_MASK) | \
		(((word_t)(num) << RMEM_BLOCK_NUM_SHIFT) & RMEM_BLOCK_NUM_MASK)            \
	)

#ifdef __NEED_RMEM_SERVER

	/**
	 * @brief Operations on remote memory.
	 */
	/**@{*/
	#define RMEM_EXIT    0 /**< Exit        */
	#define RMEM_READ    1 /**< Read        */
	#define RMEM_WRITE   2 /**< Write       */
	#define RMEM_ALLOC   3 /**< Alloc       */
	#define RMEM_MEMFREE 4 /**< Free        */
	#define RMEM_ACK     5 /**< Acknowledge */
	/**@}*/

	/**
	 * @brief Remote memory message.
	 */
	struct rmem_message
	{
		message_header header; /**< Message header. */
		rpage_t blknum;        /**< Block number.   */
		int errcode;           /**< Error code.     */
		#ifdef __RMEM_USES_MAILBOX
		char payload[RMEM_PAYLOAD_SIZE]; /**< Payload.           */
		size_t offset;                   /**< Read/Write offset. */
		#endif
	};

	/**
	 * @brief Table of RMem Servers.
	 */
	extern struct rmem_servers_info
	{
		int nodenum;
		int portnum;
		const char *name;
	} rmem_servers[RMEM_SERVERS_NUM];

#endif /* __NEED_RMEM_SERVER */

#endif /* NANVIX_SERVERS_RMEM_H_ */
