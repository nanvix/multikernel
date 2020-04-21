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

#ifndef NANVIX_CONFIG_MPPA256_H_
#define NANVIX_CONFIG_MPPA256_H_

	/**
	 * @name Number of Servers
	 */
	/**@{*/
	#define SPAWNERS_NUM      2 /**< Spawn Servers */
	#define NAME_SERVERS_NUM  1 /**< Name Servers  */
	#define RMEM_SERVERS_NUM  2 /**< RMem Servers  */
	/**@}*/

	/**
	 * @name Map of Spawn Servers
	 */
	/**@{*/
	#define SPAWN_SERVER_0_NODE 0 /**< Spawn Server 0 */
	#define SPAWN_SERVER_1_NODE 4 /**< Spawn Server 1 */
	/**@}*/

	/**
	 * @name Map of Servers
	 */
	/**@{*/
	#define NAME_SERVER_NODE   0 /**< Name Server */
	#define RMEM_SERVER_0_NODE 4 /**< RMem Server */
	#define RMEM_SERVER_1_NODE 0 /**< RMem Server */
	/**@}*/

	/**
	 * @name Map of Port Numbers
	 */
	/**@{*/
	#define NAME_SERVER_PORT_NUM   2 /**< Name Server   */
	#define RMEM_SERVER_0_PORT_NUM 2 /**< RMem Server 0 */
	#define RMEM_SERVER_1_PORT_NUM 3 /**< RMem Server 1 */
	/**@}*/

#endif /* NANVIX_CONFIG_MPPA256_H_ */
