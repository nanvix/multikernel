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

#ifndef NANVIX_SERVERS_SPAWN_H_
#define NANVIX_SERVERS_SPAWN_H_

	#include <nanvix/sys/semaphore.h>
	#include <nanvix/config.h>

	/**
	 * @name Spawn rings.
	 */
	/**@{*/
	#define SPAWN_RING_0 0
	#define SPAWN_RING_1 1
	#define SPAWN_RING_2 2
	#define SPAWN_RING_3 3
	#define SPAWN_RING_4 4
	#define SPAWN_RING_X 5
	/**@}*/

	/**
	 * @brief First spawn ring.
	 */
	#define SPAWN_RING_FIRST SPAWN_RING_0

	/**
	 * @brief Last spawn ring.
	 */
	#define SPAWN_RING_LAST SPAWN_RING_4

#ifdef SPAWN_SERVER

	/**
	 * @brief Number of spawn rings.
	 */
	#define SPAWN_RINGS_NUM SPAWN_RING_LAST

	/**
	 * @brief Declares the servers table.
	 *
	 * @param n Number of servers.
	 * @param x Servers table.
	 * @param y Spawner name.
	 */
	#define SPAWN_SERVERS(n, x, y)            \
		const int SERVERS_NUM = n;            \
		const struct serverinfo *SERVERS = x; \
		const char *spawner_name = y;

	/**
	 * @brief Server information.
	 */
	struct serverinfo
	{
		int ring;                                /**< Ring Level    */
		int (*main) (struct nanvix_semaphore *); /**< Main Function */
	};

	/**
	 * @brief Initializes the spawn barrier.
	 */
	extern void spawn_barrier_setup(void);

	/**
	 * @brief Shutdowns the spawn barrier.
	 */
	extern void spawn_barrier_cleanup(void);

	/**
	 * @brief Waits on the startup barrier
	 */
	extern void spawn_barrier_wait(void);

#endif /* SPAWN_SERVER*/

#endif /* NANVIX_SERVERS_SPAWN_H_ */
