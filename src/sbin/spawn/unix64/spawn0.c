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

#define SPAWN_SERVER

#include <nanvix/servers/spawn.h>
#include <nanvix/sys/semaphore.h>

/* Import definitions. */
extern int hello_server(struct nanvix_semaphore *);
extern int name_server(struct nanvix_semaphore *);
extern int rmem_server(struct nanvix_semaphore *);

/**
 * @brief Number of servers.
 */
#define SPAWN_SERVERS_NUM 1

/**
 * @brief Table of servers.
 */
const struct serverinfo spawn_servers[SPAWN_SERVERS_NUM] = {
	{ .ring = SPAWN_RING_0, .main = name_server },
};

SPAWN_SERVERS(SPAWN_SERVERS_NUM, spawn_servers, "spawn0")
