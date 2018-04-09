/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#ifndef NANVIX_ARCH_MPPA256
#define NANVIX_ARCH_MPPA256

	#ifndef _KALRAY_MPPA256
		#error "bad target"
	#endif

	#include <HAL/hal/core/timer.h>
	#include <HAL/hal/core/diagnostic.h>
#ifdef _KALRAY_MPPA_256_HIGH_LEVEL
	#include <mppaipc.h>
#endif
#ifdef _KALRAY_MPPA_256_LOW_LEVEL
	#include <mppa_power.h>
	#include <mppa_rpc.h>
	#include <mppa_async.h>
	#include <utask.h>
#endif

	/**
	 * @brief Number of compute clusters.
	 */
	#define NR_CCLUSTER 16

	/**
	 * @brief Number of IO clusters.
	 */
	#define NR_IOCLUSTER 2

	/**
	 * @brief Number DMAs per compute cluster.
	 */
	#define NR_CCLUSTER_DMA 1

	/**
	 * @brief Number of DMAs per compute cluster.
	 */
	#define NR_IOCLUSTER_DMA 4

	/**
	 * @brief Overall number of DMAs
	 */
	#define NR_DMA \
		(NR_CCLUSTER*NR_CCLUSTER_DMA + NR_IOCLUSTER*NR_IOCLUSTER_DMA)

	/* Cluster IDs. */
	#define CCLUSTER0    0 /**< Compute cluster  0. */
	#define CCLUSTER1    1 /**< Compute cluster  1. */
	#define CCLUSTER2    2 /**< Compute cluster  2. */
	#define CCLUSTER3    3 /**< Compute cluster  3. */
	#define CCLUSTER4    4 /**< Compute cluster  4. */
	#define CCLUSTER5    5 /**< Compute cluster  5. */
	#define CCLUSTER6    6 /**< Compute cluster  6. */
	#define CCLUSTER7    7 /**< Compute cluster  7. */
	#define CCLUSTER8    8 /**< Compute cluster  8. */
	#define CCLUSTER9    9 /**< Compute cluster  9. */
	#define CCLUSTER10  10 /**< Compute cluster 10. */
	#define CCLUSTER11  11 /**< Compute cluster 11. */
	#define CCLUSTER12  12 /**< Compute cluster 12. */
	#define CCLUSTER13  13 /**< Compute cluster 13. */
	#define CCLUSTER14  14 /**< Compute cluster 14. */
	#define CCLUSTER15  15 /**< Compute cluster 15. */
	#define IOCLUSTER0 128 /**< IO cluster 0.       */
	#define IOCLUSTER1 192 /**< IO cluster 1.       */

	/**
	 * @brief Size (in bytes) of a mailbox message.
	 */
	#define MAILBOX_MSG_SIZE 64
	
	/* Forward definitions. */
	extern int k1_get_cluster_id(void);
	extern int k1_get_cpu_id(void);
	extern int k1_is_ccluster(int);
	extern int k1_is_iocluster(int);
	extern long k1_get_ccluster_freq(void);

	/* Forward defnitions. */
	extern long k1_timer_get(void);
	extern long k1_timer_diff(long, long);
	extern void k1_timer_init(void);

#endif /* NANVIX_ARCH_MPPA256 */