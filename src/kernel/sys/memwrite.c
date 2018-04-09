/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Nanvix.
 * 
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/hal.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <stdio.h>
#include <string.h>
#include "mem.h"

/**
 * @brief Writes data to a remote memory.
 *
 * @param addr Remote address.
 * @param bug  Location where the data should be read from.
 * @param n    Number of bytes to write.
 */
void memwrite(uint64_t addr, const void *buf, size_t n)
{
	struct rmem_message msg;

	meminit();

	/* Build operation header. */
	msg.source = k1_get_cluster_id();
	msg.op = RMEM_WRITE;
	msg.blknum = addr;
	msg.size = n;

	printf("send operation header()\n");
	/* Send operation header. */
	mailbox_write(_mem_outbox, &msg);
	printf("send data\n");

	/* Send data. */
	portal_write(_mem_outportal, buf, n);
}
