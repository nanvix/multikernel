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

#define __NEED_RMEM_SERVICE
#define __NEED_RMEM_SERVER

#include <nanvix/servers/message.h>
#include <nanvix/runtime/pm.h>
#include <nanvix/servers/rmem.h>
#include <nanvix/runtime/utils.h>
#include <nanvix/sys/thread.h>
#include <nanvix/sys/page.h>
#include <nanvix/sys/perf.h>
#include <nanvix/sys/semaphore.h>
#include <nanvix/limits.h>
#include <nanvix/types.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/stdint.h>

/**
 * @brief Bad rmeote geometry for remote memory?
 */
#if (RMEM_NUM_BLOCKS%(BITMAP_WORD_LENGTH/8) != 0)
#error "bad geometry for remote memory"
#endif

/**
 * @brief Debug RMEM?
 */
#define __DEBUG_RMEM 0

#if (__DEBUG_RMEM)
	#define rmem_debug(fmt, ...) uprintf(fmt, __VA_ARGS__)
#else
	#define rmem_debug(fmt, ...) { }
#endif

/**
 * @brief Server statistics.
 */
static struct
{
	unsigned nallocs;   /**< Number of allocations. */
	unsigned nfrees;    /**< Number of frees.       */
	unsigned nreads;    /**< Number of reads.       */
	unsigned nwrites;   /**< Number of writes.      */
	uint64_t tstart;    /**< Start time.            */
	uint64_t tshutdown; /**< Shutdown time.         */
	uint64_t talloc;    /**< Allocation time.       */
	uint64_t tfree;     /**< Free time.             */
	uint64_t tread;     /**< Read time.             */
	uint64_t twrite;    /**< Write time.            */
	unsigned nblocks;   /**< Blocks allocated       */
} stats = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

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
 * @brief Server ID
 */
static int serverid;

/**
 * @brief Remote memory.
 *
 * @todo TODO: allocate this dynamically with kernel calls.
 */
static struct
{
	char *blocks;                                        /**< Blocks         */
	nanvix_pid_t owners[RMEM_NUM_BLOCKS];                /**< Owners         */
	bitmap_t bitmap[RMEM_NUM_BLOCKS/BITMAP_WORD_LENGTH]; /**< Allocation Map */
} rmem;

/**
 * @brief Map of blocks.
 */

/*============================================================================*
 * rmem_server_get_name()                                                     *
 *============================================================================*/

/**
 * @brief Gets the name of this server.
 */
static const char *rmem_server_get_name(void)
{
	/* Search for server. */
	for (int i = 0; i < RMEM_SERVERS_NUM; i++)
	{
		/* Found. */
		if (nodenum == rmem_servers[i].nodenum)
			return (rmem_servers[i].name);
	}

	/* Should not happen. */
	return (NULL);
}

/**
 * @brief Gets the ID of this server.
 */
static int rmem_server_get_id(void)
{
	/* Search for server. */
	for (int i = 0; i < RMEM_SERVERS_NUM; i++)
	{
		/* Found. */
		if (nodenum == rmem_servers[i].nodenum)
			return (i);
	}

	/* Should not happen. */
	return (-1);
}

/*============================================================================*
 * do_rmem_alloc()                                                            *
 *============================================================================*/

/**
 * @brief Handles remote memory allocation.
 *
 * @param owner Owner of the block.
 *
 * @returns Upon successful completion, the number of the newly
 * allocated remote memory block is allocated. Upon failure, @p
 * RMEM_NULL is returned instead.
 */
static inline rpage_t do_rmem_alloc(nanvix_pid_t owner)
{
	bitmap_t bit;

	/* Memory server is full. */
	if (stats.nblocks == RMEM_NUM_BLOCKS)
	{
		uprintf("[nanvix][rmem] remote memory full");
		return (RMEM_NULL);
	}

	/* Find a free block. */
	uassert(
		(bit = bitmap_first_free(
			rmem.bitmap,
			(RMEM_NUM_BLOCKS/BITMAP_WORD_LENGTH)*sizeof(bitmap_t)
		)) != BITMAP_FULL
	);

	/* Allocate block. */
	stats.nblocks++;
	bitmap_set(rmem.bitmap, bit);
	rmem.owners[bit] = owner;
	rmem_debug("rmem_alloc() blknum=%d nblocks=%d/%d",
		bit, stats.nblocks, RMEM_NUM_BLOCKS
	);

	return (RMEM_BLOCK(serverid, bit));
}

/*============================================================================*
 * do_rmem_free()                                                             *
 *============================================================================*/

/**
 * @brief Handles remote memory free.
 *
 * @param blknum Number of the target block.
 * @param owner Owner of the target block.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static inline int do_rmem_free(rpage_t blknum, nanvix_pid_t owner)
{
	rpage_t _blknum;

	_blknum = RMEM_BLOCK_NUM(blknum);

	/* Invalid block number. */
	if ((_blknum == RMEM_NULL) || (_blknum >= RMEM_NUM_BLOCKS))
	{
		uprintf("[nanvix][rmem] invalid block number");
		return (-EINVAL);
	}

	/* Remote memory is empty. */
	if (stats.nblocks == 1)
	{
		uprintf("[nanvix][rmem] remote memory is empty");
		return (-EFAULT);
	}

	/* Bad block number. */
	if (!bitmap_check_bit(rmem.bitmap, _blknum))
	{
		uprintf("[nanvix][rmem] bad free block");
		return (-EFAULT);
	}

	/* Memory violation. */
	if (rmem.owners[_blknum] != owner)
	{
		uprintf("[nanvix][rmem] memory violation");
		return (-EFAULT);
	}

	/* Clean block. */
	umemset(&rmem.blocks[_blknum*RMEM_BLOCK_SIZE], 0, RMEM_BLOCK_SIZE);

	/* Free block. */
	stats.nblocks--;
	bitmap_clear(rmem.bitmap, _blknum);
	rmem_debug("rmem_free() blknum=%d nblocks=%d/%d",
		_blknum, stats.nblocks, RMEM_NUM_BLOCKS
	);

	return (0);
}

/*============================================================================*
 * do_rmem_write()                                                            *
 *============================================================================*/

#ifndef __RMEM_USES_MAILBOX

/**
 * @brief Handles a write request.
 *
 * @param remote Remote client.
 * @param blknum Number of the target block.
 */
static inline int do_rmem_write(int remote, rpage_t blknum, int remote_port)
{
	int ret = 0;
	rpage_t _blknum;

	rmem_debug("write() nodenum=%d blknum=%x",
		remote,
		blknum
	);

	_blknum = RMEM_BLOCK_NUM(blknum);

	/* Invalid block number. */
	if ((_blknum == RMEM_NULL) || (_blknum >= RMEM_NUM_BLOCKS))
	{
		uprintf("[nanvix][rmem] invalid block number");
		return (-EINVAL);
	}

	/*
	 * Bad block number. Drop this read and return
	 * an error. Note that we use the NULL block for this.
	 */
	if (!bitmap_check_bit(rmem.bitmap, _blknum))
	{
		uprintf("[nanvix][rmem] bad write block");
		_blknum = 0;
		ret = -EFAULT;
	}

	uassert(kportal_allow(inportal, remote, remote_port) == 0);
	uassert(
		kportal_read(
			inportal,
			&rmem.blocks[_blknum*RMEM_BLOCK_SIZE],
			RMEM_BLOCK_SIZE
		) == RMEM_BLOCK_SIZE
	);

	return (ret);
}

#else

/**
 * @brief Handles a write request.
 *
 * @param remote Remote client.
 * @param blknum Number of the target block.
 */
static inline int do_rmem_write(rpage_t blknum, size_t offset, const char *payload)
{
	int ret = 0;
	rpage_t _blknum;

	rmem_debug("write() nodenum=%d blknum=%x",
		remote,
		blknum
	);

	_blknum = RMEM_BLOCK_NUM(blknum);

	/* Invalid block number. */
	if ((_blknum == RMEM_NULL) || (_blknum >= RMEM_NUM_BLOCKS))
	{
		uprintf("[nanvix][rmem] invalid block number");
		return (-EINVAL);
	}

	/*
	 * Bad block number. Drop this read and return
	 * an error. Note that we use the NULL block for this.
	 */
	if (!bitmap_check_bit(rmem.bitmap, _blknum))
	{
		uprintf("[nanvix][rmem] bad write block");
		_blknum = 0;
		ret = -EFAULT;
	}

	umemcpy(
		&rmem.blocks[_blknum*RMEM_BLOCK_SIZE + offset],
		payload,
		RMEM_PAYLOAD_SIZE
	);

	return (ret);
}

#endif

/*============================================================================*
 * do_rmem_read()                                                             *
 *============================================================================*/

#ifndef __RMEM_USES_MAILBOX

/**
 * @brief Handles a read request.
 *
 * @param remote Remote client.
 * @param blknum Number of the target block.
 * @param outbox Output mailbox to remote client.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static inline int do_rmem_read(int remote, rpage_t blknum, int outbox, int outport)
{
	int ret = 0;
	int outportal;
	rpage_t _blknum;
	struct rmem_message msg;

	/* Build operation header. */
	msg.header.source = knode_get_num();
	msg.header.opcode = RMEM_ACK;

	rmem_debug("read() nodenum=%d blknum=%x",
		remote,
		blknum
	);

	_blknum = RMEM_BLOCK_NUM(blknum);

	/* Invalid block number. */
	if ((_blknum == RMEM_NULL) || (_blknum >= RMEM_NUM_BLOCKS))
	{
		uprintf("[nanvix][rmem] invalid block number");
		return (-EINVAL);
	}

	/*
	 * Bad block number. Let us send a null block
	 * and return an error instead.
	 */
	if (!bitmap_check_bit(rmem.bitmap, _blknum))
	{
		uprintf("[nanvix][rmem] bad read block");
		_blknum = 0;
		ret = -EFAULT;
	}

	uassert((outportal =
		kportal_open(
			knode_get_num(),
			remote,
			outport)
		) >= 0
	);
	msg.header.portal_port = kcomm_get_port(outportal, COMM_TYPE_PORTAL);
	uassert(
		kmailbox_write(outbox,
			&msg,
			sizeof(struct rmem_message)
		) == sizeof(struct rmem_message)
	);
	uassert(
		kportal_write(
			outportal,
			&rmem.blocks[_blknum*RMEM_BLOCK_SIZE],
			RMEM_BLOCK_SIZE
		) == RMEM_BLOCK_SIZE
	);
	uassert(kportal_close(outportal) == 0);

	return (ret);
}

#else

/**
 * @brief Handles a read request.
 *
 * @param remote Remote client.
 * @param blknum Number of the target block.
 * @param outbox Output mailbox to remote client.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static inline int do_rmem_read(rpage_t blknum, int outbox)
{
	int ret = 0;
	rpage_t _blknum;
	struct rmem_message msg;

	/* Build operation header. */
	msg.header.source = knode_get_num();
	msg.header.opcode = RMEM_ACK;
	msg.blknum = blknum;

	rmem_debug("read() nodenum=%d blknum=%x",
		remote,
		blknum
	);

	_blknum = RMEM_BLOCK_NUM(blknum);

	/* Invalid block number. */
	if ((_blknum == RMEM_NULL) || (_blknum >= RMEM_NUM_BLOCKS))
	{
		uprintf("[nanvix][rmem] invalid block number");
		return (-EINVAL);
	}

	/*
	 * Bad block number. Let us send a null block
	 * and return an error instead.
	 */
	if (!bitmap_check_bit(rmem.bitmap, _blknum))
	{
		uprintf("[nanvix][rmem] bad read block");
		_blknum = 0;
		ret = -EFAULT;
	}

	for (size_t i = 0; i < RMEM_BLOCK_SIZE; i += RMEM_PAYLOAD_SIZE)
	{
		msg.offset = i;

		umemcpy(
			&msg.payload,
			&rmem.blocks[_blknum*RMEM_BLOCK_SIZE + i],
			RMEM_PAYLOAD_SIZE
		);

		uassert(
			kmailbox_write(
				outbox,
				&msg, sizeof(struct rmem_message)
			) == sizeof(struct rmem_message)
		);
	}

	return (ret);
}

#endif

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
	uint64_t t0, t1;

	kclock(&stats.tstart);

	while(!shutdown)
	{
		int source;
		struct rmem_message msg;

		uassert(
			kmailbox_read(
				inbox,
				&msg,
				sizeof(struct rmem_message)
			) == sizeof(struct rmem_message)
		);

		rmem_debug("rmem request source=%d port=%d opcode=%d",
			msg.header.source,
			msg.header.portal_port,
			msg.header.opcode
		);

		/* handle write operation. */
		switch (msg.header.opcode)
		{
			/* Write to RMEM. */
			case RMEM_WRITE:
				stats.nwrites++;
				kclock(&t0);
					#ifndef __RMEM_USES_MAILBOX
					msg.errcode = do_rmem_write(msg.header.source, msg.blknum, msg.header.portal_port);
					#else
					msg.errcode = do_rmem_write(msg.blknum, msg.offset, msg.payload);
					#endif
					uassert((source = kmailbox_open(msg.header.source, msg.header.mailbox_port)) >= 0);
					uassert(kmailbox_write(source, &msg, sizeof(struct rmem_message)) == sizeof(struct rmem_message));
					uassert(kmailbox_close(source) == 0);
				kclock(&t1);
				stats.twrite += (t1 - t0);
				break;

			/* Read a page. */
			case RMEM_READ:
				stats.nreads++;
				kclock(&t0);
					uassert((source = kmailbox_open(msg.header.source, msg.header.mailbox_port)) >= 0);
					#ifndef __RMEM_USES_MAILBOX
					msg.errcode = do_rmem_read(msg.header.source, msg.blknum, source, msg.header.portal_port);
					#else
					msg.errcode = do_rmem_read(msg.blknum, source);
					#endif
					uassert(kmailbox_write(source, &msg, sizeof(struct rmem_message)) == sizeof(struct rmem_message));
					uassert(kmailbox_close(source) == 0);
				kclock(&t1);
				stats.tread += (t1 - t0);
				break;

			/* Allocates a page. */
			case RMEM_ALLOC:
				stats.nallocs++;
				kclock(&t0);
					msg.blknum = do_rmem_alloc(msg.header.source);
					msg.errcode = (msg.blknum == RMEM_NULL) ? RMEM_NULL : msg.blknum;
					uassert((source = kmailbox_open(msg.header.source, msg.header.mailbox_port)) >= 0);
					uassert(kmailbox_write(source, &msg, sizeof(struct rmem_message)) == sizeof(struct rmem_message));
					uassert(kmailbox_close(source) == 0);
				kclock(&t1);
				stats.talloc += (t1 - t0);
			    break;

			/* Free frees a page. */
			case RMEM_MEMFREE:
				stats.nfrees++;
				kclock(&t0);
					msg.errcode = do_rmem_free(msg.blknum, msg.header.source);
					uassert((source = kmailbox_open(msg.header.source, msg.header.mailbox_port)) >= 0);
					uassert(kmailbox_write(source, &msg, sizeof(struct rmem_message)) == sizeof(struct rmem_message));
					uassert(kmailbox_close(source) == 0);
				kclock(&t1);
				stats.tfree += (t1 - t0);
			    break;

			case RMEM_EXIT:
				kclock(&stats.tshutdown);
				shutdown = 1;
				break;

			/* Should not happen. */
			default:
				break;
		}
	}

	/* Dump statistics. */
	uprintf("[nanvix][rmem] nallocs=%d nfrees=%d nreads=%d nwrites=%d",
			stats.nallocs, stats.nfrees,
			stats.nreads, stats.nwrites
	);

	return (0);
}

/*============================================================================*
 * do_rmem_startup()                                                          *
 *============================================================================*/

/**
 * @brief Start address of remote memory area.
 */
#define RMEM_START UBASE_VIRT

/**
 * @brief End address of remote memory area.
 */
#define RMEM_END (UBASE_VIRT + RMEM_SIZE)

/**
 * @brief Initializes the remote memory server.
 *
 * @returns Upon successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
static int do_rmem_startup(struct nanvix_semaphore *lock)
{
	int ret;
	const char *servername;

	/* Messages should be small enough. */
	uassert(sizeof(struct rmem_message) <= NANVIX_MAILBOX_MESSAGE_SIZE);

	/* Payload should have a good size. */
#ifdef __RMEM_USES_MAILBOX
	uassert((RMEM_BLOCK_SIZE%RMEM_PAYLOAD_SIZE) == 0);
#endif

	/* Bitmap word should be large enough. */
	uassert(sizeof(rpage_t) >= sizeof(bitmap_t));

	/* Physical memory should be big enough. */
	uassert(RMEM_SIZE <=  UMEM_SIZE);
	uassert((RMEM_SIZE%PAGE_SIZE) == 0);

	/* Allocate physical memory. */
	rmem.blocks = (char *) RMEM_START;
	for (vaddr_t vaddr = RMEM_START; vaddr < RMEM_END; vaddr += PAGE_SIZE)
		uassert(page_alloc(vaddr) == 0);

	/* Clean bitmap. */
	umemset(
		rmem.bitmap,
		0,
		(RMEM_NUM_BLOCKS/BITMAP_WORD_LENGTH)*sizeof(bitmap_t)
	);

	/* Fist block is special. */
	stats.nblocks++;
	bitmap_set(rmem.bitmap, 0);

	/* Clean all blocks. */
	for (unsigned long i = 0; i < RMEM_NUM_BLOCKS; i++)
		umemset(&rmem.blocks[i*RMEM_BLOCK_SIZE], 0, RMEM_BLOCK_SIZE);

	nodenum = knode_get_num();

	/* Assign input mailbox. */
	inbox = stdinbox_get();

	/* Assign input portal. */
	inportal = stdinportal_get();

	serverid = rmem_server_get_id();

	/* Link name. */
	servername = rmem_server_get_name();
	if ((ret = name_link(nodenum, servername)) < 0)
		return (ret);

	/* Unblock spawner. */
	uprintf("[nanvix][rmem] server alive");
	uprintf("[nanvix][rmem] attached to node %d", knode_get_num());
	uprintf("[nanvix][rmem] listening to mailbox %d", inbox);
	uprintf("[nanvix][rmem] listening to portal %d", inportal);
	uprintf("[nanvix][rmem] memory size %d KB", RMEM_SIZE/KB);

	nanvix_semaphore_up(lock);

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
static int do_rmem_server(struct nanvix_semaphore *lock)
{
	int ret;

	uprintf("[nanvix][rmem] booting up server");

	if ((ret = do_rmem_startup(lock)) < 0)
	{
		uprintf("[nanvix][rmem] failed to startup server!");
		goto error;
	}

	/* Unblock spawner. */
	uprintf("[nanvix][rmem] server alive");

	if ((ret = do_rmem_loop()) < 0)
	{
		uprintf("[nanvix][rmem] failed to launch server!");
		goto error;
	}

	uprintf("[nanvix][rmem] shutting down server");

	if ((ret = do_rmem_shutdown()) < 0)
	{
		uprintf("[nanvix][rmem] failed to shutdown server!");
		goto error;
	}

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
 * @returns Always returns zero.
 */
int rmem_server(struct nanvix_semaphore *lock)
{
	uassert(do_rmem_server(lock) == 0);

	return (0);
}
