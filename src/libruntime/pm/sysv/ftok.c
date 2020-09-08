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

#include <posix/errno.h>
#include <nanvix/ulib.h>
#include <nanvix/servers/sysv/ftok.h>

/**
 * @brief Dummy function to act as replacement for stat.
 *
 * @param path Any string.
 * 
 * @returns A nanvix_key_t containing the bits of the first chars
 * of path (at most three).
 */
static nanvix_key_t stat(const char *path)
{
	nanvix_key_t key = 0;
	const int length = ustrlen(path);

	const int max = length > 3 ? 3 : length;

	for (int i = 0; i != max; ++i)
	{
		key |= path[i];
		key <<= 8;
	}

	return key;
}

/** 
 * @brief Generates an IPC key.
 * 
 * @param path Path to a file that will be used to generate an IPC key.
 * @param id ID (first 8 bits) that will be used to generate an IPC key.
 *
 * @returns Upon successfull completion, zero is returned.
 *
 * @retval -EACCES Search permission is denied for a component of the path prefix.
 * @retval -EIO	 An error occurred while reading from the file system.
 * @retval -ELOOP A loop exists in symbolic links encountered during resolution of path.
 * @retval -ENAMETOOLONG The length of a component of a pathname is longer than {NAME_MAX}
 * @retval -ENOENT A component of path does not name an existing file or path is an empty str.
 * @retval -ENOTDIR
 *
 * TODO: These errors will probably be caught by stat... For now, no error might happen
 * except -ENOENT with an empty path.
 * */
nanvix_key_t ftok(const char *path, int id)
{
	if (path == NULL || *path == '\0')
		return (-ENOENT);

	nanvix_key_t ipc_key = stat(path) | (id & 0xFF);

	return ipc_key;
}
