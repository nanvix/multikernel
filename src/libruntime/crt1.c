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

#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/ulib.h>
#include <nanvix/pm.h>

/**
 * User-level main routine.
 */
extern int __main3(int argc, const char *argv[]);


#if __NANVIX_USES_LWMPI

	/**
	 * Routine that prepare the MPI Runtime environment.
	 */
	extern int __mpi_processes_init(int(*fn)(int, const char *[]), int argc, const char *argv[]);

	/**
	 * Routine that cleanup the MPI Runtime environment.
	 */
	extern int __mpi_processes_finalize(void);

#endif


/**
 * @brief Entry point for user-level program.
 */
int __main2(int argc, const char *argv[])
{
	int nodenum;
	char pname[NANVIX_PROC_NAME_MAX];

	nodenum = knode_get_num();
	usprintf(pname, "cluster%d", nodenum);

	__runtime_setup(SPAWN_RING_FIRST);

		/* Unblock spawners. */
		uassert(stdsync_fence() == 0);
		uassert(stdsync_fence() == 0);

		__runtime_setup(SPAWN_RING_LAST);

		uassert(nanvix_setpname(pname) == 0);

		/* Spawn multiple threads simulating user processes. */
#if __NANVIX_USES_LWMPI

		uprintf("INITIALIZING MPI USER PROCESSES");

		uassert(__mpi_processes_init(&__main3, argc, argv) == 0);

#endif

		uassert(stdsync_fence() == 0);

		__main3(argc, argv);

		/* Join the user processes. */
#if __NANVIX_USES_LWMPI

		uprintf("JOINING MPI USER PROCESSES");

		uassert(__mpi_processes_finalize() == 0);

#endif

		uassert(nanvix_name_unlink(pname) == 0);
		uassert(stdsync_fence() == 0);

		nanvix_shutdown();

	__runtime_cleanup();

	return (0);
}

