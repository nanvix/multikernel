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

#include <mppa/osconfig.h>
#include <mppaipc.h>
#include <nanvix/arch/mppa.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/hal.h>
#include <nanvix/klib.h>
#include <nanvix/name.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief ID of slave processes.
 */
static int pids[NR_CCLUSTER];

/**
 * @brief API Test: master name deletion.
 */
static void test_name_remove(void)
{
	char pathname[PROC_NAME_MAX];

	/* IO cluster registration test. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
	{
		sprintf(pathname, "/name%d", i);

		/* Remove name. */
		name_unlink(pathname);
		assert(name_lookup(pathname) == (-ENOENT));
	}
}

/**
 * @brief API Test: master name registration.
 */
static void test_name_register(void)
{
	int clusterid;
	char pathname[PROC_NAME_MAX];

	clusterid = hal_get_cluster_id();

	/* IO cluster registration test. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
	{
		sprintf(pathname, "/name%d", i);

		/* Register name. */
		name_link(clusterid + i, pathname);
	}
}

/**
 * @brief API Test: master name lookup.
 */
static void test_name_lookup(void)
{
	int clusterid;
	char pathname[PROC_NAME_MAX];

	clusterid = hal_get_cluster_id();

	/* IO cluster registration test. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
	{
		sprintf(pathname, "/name%d", i);

		assert(name_lookup(pathname) == clusterid + i);
	}
}

/**
 * @brief API Test: slave name registration.
 */
void test_name_slave(int nclusters)
{
	char nclusters_str[4];
	const char *args[] = {
		"name-slave",
		nclusters_str,
		NULL
	};

	sprintf(nclusters_str, "%d", nclusters);

	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, args[0], args, NULL)) != -1);

	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}

/*===================================================================*
 * Kernel                                                            *
 *===================================================================*/

/**
 * @brief Querying the name server.
 */
int main(int argc, char **argv)
{
	int global_barrier;
	int nclusters;

	assert(argc == 2);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);

	/* Wait name server. */
	global_barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(global_barrier);

	test_name_register();
	test_name_lookup();
	test_name_remove();
	test_name_slave(nclusters);

	/* House keeping. */
	barrier_close(global_barrier);

	return (EXIT_SUCCESS);
}
