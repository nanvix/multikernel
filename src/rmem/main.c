/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
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

#define __RMEM_SERVICE
#define __NEED_NAME_CLIENT

#include <nanvix/servers/message.h>
#include <nanvix/servers/name.h>
#include <nanvix/servers/rmem.h>
#include <nanvix/servers/spawn.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/utils.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/noc.h>
#include <nanvix/sys/portal.h>
#include <nanvix/limits.h>
#include <ulibc/assert.h>
#include <ulibc/stdio.h>
#include <ulibc/stdlib.h>
#include <ulibc/string.h>
#include <posix/errno.h>
#include <stdint.h>

/**
 * @brief Debug RMEM?
 */
#define __DEBUG_RMEM 0

#if (__DEBUG_RMEM)
	#define rmem_debug(fmt, ...) debug("rmem", fmt, __VA_ARGS__)
#else
	#define rmem_debug(fmt, ...) { }
#endif

/**
 * @brief Server statistics.
 */
static struct
{
	int nreads;       /**< Number of reads.         */
	size_t read;      /**< Number of bytes read.    */
	int nwrites;      /**< Number of writes.        */
	size_t written;   /**< Number of bytes written. */
} stats = { 0, 0, 0, 0 };

/**
 * @brief Node number.
 */
static int nodenum;

/**
 * @brief Input mailbox for small messages.
 */
static int inbox;

/**
 * @brief Input portal for receiving data.
 */
static int inportal;

/**
 * @brief Remote memory.
 *
 * @todo TODO: allocate this dynamically with kernel calls.
 */
static char rmem[RMEM_NUM_BLOCKS][RMEM_BLOCK_SIZE];

/**
 * @brief Map of blocks.
 */
static bitmap_t blocks[RMEM_NUM_BLOCKS/BITMAP_WORD_LENGTH];

/*============================================================================*
 * do_rmem_alloc()                                                            *
 *============================================================================*/

/**
 * @brief Handles remote memory allocation.
 *
 * @returns Upon successful completion, the number of the newly
 * allocated remote memory block is allocated. Upon failure, @p
 * RMEM_NULL is returned instead.
 */
static inline rpage_t do_rmem_alloc(void)
{
	bitmap_t bit;

	/* Find a free block. */
	bit = bitmap_first_free(
		blocks,
		(RMEM_NUM_BLOCKS/BITMAP_WORD_LENGTH)*sizeof(bitmap_t)
	);

	/* Remote memory bank is full. */
	if (bit == BITMAP_FULL)
	{
        nanvix_printf("[nanvix][rmem] remote memory full\n");
		return (RMEM_NULL);
	}

	/* Allocate block. */
	rmem_debug("rmem_alloc() blknum=%d", bit);
    bitmap_set(blocks, bit);

	return (bit);
}

/*============================================================================*
 * do_rmem_free()                                                             *
 *============================================================================*/

/**
 * @brief Handles remote memory free.
 *
 * @param blknum Number of the target block.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static inline int do_rmem_free(rpage_t blknum)
{
	rmem_debug("rmem_free blknum=%d", blknum);

	/* Invalid block number. */
	if ((blknum == RMEM_NULL) || (blknum >= RMEM_NUM_BLOCKS))
    {
        nanvix_printf("[nanvix][rmem] invalid block number\n");
        return (-EINVAL);
    }

	/* Bad block number. */
    if (!bitmap_check_bit(blocks, blknum))
    {
        nanvix_printf("[nanvix][rmem] bad free block\n");
        return (-EFAULT);
    }

	/* Free block. */
	bitmap_clear(blocks, blknum);

	return (0);
}

/*============================================================================*
 * do_rmem_write()                                                            *
 *============================================================================*/

/**
 * @brief Handles a write request.
 *
 * @param remote Remote client.
 * @param blknum Number of the target block.
 */
static inline int do_rmem_write(int remote, rpage_t blknum)
{
	rmem_debug("write nodenum=%d blknum=%d",
		remote,
		blknum
	);

	/*
	 * FIXME: we should send an extra message to say what the remote
	 * should do: either send data or abort.
	 */

	/* Invalid block number. */
	if ((blknum == RMEM_NULL) || (blknum >= RMEM_NUM_BLOCKS))
    {
        nanvix_printf("[nanvix][rmem] invalid block number\n");
        return (-EINVAL);
    }

	/* Bad block number. */
    if (!bitmap_check_bit(blocks, blknum))
    {
        nanvix_printf("[nanvix][rmem] bad write block\n");
        return (-EFAULT);
    }

	nanvix_assert(kportal_allow(inportal, remote) == 0);
	nanvix_assert(
		kportal_read(
			inportal,
			&rmem[blknum][0],
			RMEM_BLOCK_SIZE
		) == RMEM_BLOCK_SIZE
	);

	return (0);
}

/*============================================================================*
 * do_rmem_read()                                                             *
 *============================================================================*/

/**
 * @brief Handles a read request.
 *
 * @param remote Remote client.
 * @param blknum Number of the target block.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static inline int do_rmem_read(int remote, rpage_t blknum)
{
	int outportal;

	rmem_debug("read nodenum=%d blknum=%d",
		remote,
		blknum
	);

	/*
	 * FIXME: we should send an extra message to say what the remote
	 * should do: either wait for data or abort.
	 */

	/* Invalid block number. */
	if ((blknum == RMEM_NULL) || (blknum >= RMEM_NUM_BLOCKS))
    {
        nanvix_printf("[nanvix][rmem] invalid block number\n");
        return (-EINVAL);
    }

	/* Bad block number. */
    if (!bitmap_check_bit(blocks, blknum))
    {
        nanvix_printf("[nanvix][rmem] bad read block\n");
        return (-EFAULT);
    }

	nanvix_assert((outportal =
		kportal_open(
			knode_get_num(),
			remote)
		) >= 0
	);
	nanvix_assert(
		kportal_write(
			outportal,
			&rmem[blknum][0],
			RMEM_BLOCK_SIZE
		) == RMEM_BLOCK_SIZE
	);
	nanvix_assert(kportal_close(outportal) == 0);

	return (0);
}

/*============================================================================*
 * do_rmem_loop()                                                             *
 *============================================================================*/

/**
 * @brief Handles remote memory requests.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_rmem_loop(void)
{
	int shutdown = 0;
    int source;

	while(!shutdown)
	{
		struct rmem_message msg;

		nanvix_assert(
			kmailbox_read(
				inbox,
				&msg,
				sizeof(struct rmem_message)
			) == sizeof(struct rmem_message)
		);

		rmem_debug("rmem request source=%d opcode=%d",
			msg.header.source,
			msg.header.opcode
		);

		/* handle write operation. */
		switch (msg.header.opcode)
		{
			/* Write to RMEM. */
			case RMEM_WRITE:
				stats.nwrites++;
				stats.written += RMEM_BLOCK_SIZE;
				shutdown = (do_rmem_write(msg.header.source, msg.blknum) < 0) ? 1 : 0;
				break;

			/* Read a page. */
			case RMEM_READ:
				stats.nreads++;
				stats.read += RMEM_BLOCK_SIZE;
				shutdown = (do_rmem_read(msg.header.source, msg.blknum) < 0) ? 1 : 0;
				break;

            /* Allocates a page. */
            case RMEM_ALLOC:
                msg.blknum = do_rmem_alloc();
				msg.errcode = (msg.blknum == RMEM_NULL) ? -ENOMEM : 0;
                nanvix_assert((source = kmailbox_open(msg.header.source)) >= 0);
                nanvix_assert(kmailbox_write(source, &msg, sizeof(struct rmem_message)) == sizeof(struct rmem_message));
                nanvix_assert(kmailbox_close(source) == 0);
                break;

            /* Free frees a page. */
            case RMEM_MEMFREE:
				msg.errcode = do_rmem_free(msg.blknum);
                nanvix_assert((source = kmailbox_open(msg.header.source)) >= 0);
                nanvix_assert(kmailbox_write(source, &msg, sizeof(struct rmem_message)) == sizeof(struct rmem_message));
                nanvix_assert(kmailbox_close(source) == 0);
                break;

			case RMEM_EXIT:
				shutdown = 1;
				break;

			/* Should not happen. */
			default:
				break;
		}
	}

	return (0);
}

/*============================================================================*
 * do_rmem_startup()                                                          *
 *============================================================================*/

/**
 * @brief Initializes the remote memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_rmem_startup(void)
{
	int ret;
	char pathname[NANVIX_PROC_NAME_MAX];

	/* Messages should be small enough. */
	nanvix_assert(sizeof(struct rmem_message) <= MAILBOX_MSG_SIZE);

	/* Bitmap word should be large enough. */
	nanvix_assert(sizeof(rpage_t) >= sizeof(bitmap_t));

	/* Clean bitmap. */
    nanvix_memset(
		blocks,
		0,
		(RMEM_NUM_BLOCKS/BITMAP_WORD_LENGTH)*sizeof(bitmap_t)
	);

	/* Fist block is special. */
	bitmap_set(blocks, 0);

	nodenum = knode_get_num();

	/* Assign input mailbox. */
	inbox = stdinbox_get();

	/* Assign input portal. */
	inportal = stdinportal_get();

	/* Link name. */
	nanvix_strcpy(pathname, "/rmem");
	if ((ret = name_link(nodenum, pathname)) < 0)
		return (ret);

	return (0);
}

/*============================================================================*
 * do_rmem_shutdown()                                                         *
 *============================================================================*/

/**
 * @brief Shutdowns the remote memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_rmem_shutdown(void)
{
	return (0);
}

/*============================================================================*
 * do_rmem_server()                                                           *
 *============================================================================*/

/**
 * @brief Remote memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int do_rmem_server(void)
{
	int ret;

	nanvix_printf("[nanvix][rmem] booting up server\n");

	if ((ret = do_rmem_startup()) < 0)
		goto error;

	/* Unblock spawner. */
	nanvix_assert(stdsync_fence() == 0);
	nanvix_printf("[nanvix][rmem] server alive\n");

	if ((ret = do_rmem_loop()) < 0)
		goto error;

	nanvix_printf("[nanvix][rmem] shutting down server\n");

	if ((ret = do_rmem_shutdown()) < 0)
		goto error;

	return (0);

error:
	return (ret);
}

/*============================================================================*
 * __main2()                                                                  *
 *============================================================================*/

/**
 * @brief Handles remote memory requests.
 *
 * @param argc Argument count (unused).
 * @param argv Argument list (unused).
 *
 * @returns Always returns zero.
 */
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	__runtime_setup(1);

	do_rmem_server();

	__runtime_cleanup();

	return (0);
}
