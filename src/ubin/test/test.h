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

#ifndef _TEST_H_
#define _TEST_H_

	#include <nanvix/ulib.h>

	/**
	 * @brief Asserts a logic expression.
	 */
	#define TEST_ASSERT(x) uassert(x)

	/**
	 * @brief Unit test.
	 */
	struct test
	{
		void (*test_fn)(void); /**< Test function. */
		const char *name;      /**< Test name.     */
	};

	/**
	 * @brief Launches regression tests on Name Service.
	 */
	extern void test_name(void);

	/**
	 * @brief Launches regression tests on RMem Manager.
	 */
	extern void test_rmem_stub(void);

	/**
	 * @brief Launches regression tests on RMem Cache.
	 */
	extern void test_rmem_cache(void);

	/**
	 * @brief Launches regression tests on RMem Interface.
	 */
	extern void test_rmem_manager(void);

	/**
	 * @brief Launches regression tests on SHM Service.
	 */
	extern void test_shm(void);

	/**
	 * @brief Launches regression tests on POSIX manager.
	 */
	extern void test_posix(void);

	/**
	 * @brief Launches regression tets on System V Service.
	 */
	extern void test_sysv(void);

	/**
	 * @brief Launches regression tests on VFS manager.
	 */
	extern void test_vfs(void);

	/**
	 * @brief Launches regression tests on process/group id manager.
	 */
	extern void test_proc(void);

	/**
	 * @brief Horizontal line for tests.
	 */
	extern const char *HLINE;

#endif /* _TEST_H_ */
