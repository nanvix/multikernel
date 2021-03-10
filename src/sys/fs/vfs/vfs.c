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

/* Must come first. */
#define __VFS_SERVER

#include <nanvix/servers/vfs.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>
#include <posix/fcntl.h>

/*============================================================================*
 * vfs_open()                                                                 *
 *============================================================================*/

/**
 * @see fs_open().
 */
int vfs_open(int connection, const char *filename, int oflag, mode_t mode)
{
	/* Invalid file name. */
	if (filename == NULL)
		return (-EINVAL);

	/* Launch process. */
	if (fprocess_launch(connection) < 0)
		return (-EINVAL);

	return (fs_open(filename, oflag, mode));
}

/*============================================================================*
 * vfs_stat()                                                                 *
 *============================================================================*/

/**
 * @see fs_stat().
 */
int vfs_stat(int connection, const char *filename, struct nanvix_stat *restrict buf)
{
	/* Invalid file name. */
	if (filename == NULL)
		return (-EINVAL);

	/* Launch process. */
	if (fprocess_launch(connection) < 0)
		return (-EINVAL);

	return (fs_stat(filename, buf));
}

/*============================================================================*
 * vfs_close()                                                                *
 *============================================================================*/

/**
 * @see fs_close().
 */
int vfs_close(int connection, int fd)
{
	/* Invalid file descriptor. */
	if (!WITHIN(fd, 0, NANVIX_OPEN_MAX))
		return (-EINVAL);

	/* Launch process. */
	if (fprocess_launch(connection) < 0)
		return (-EINVAL);

	return (fs_close(fd));
}

/*============================================================================*
 * vfs_unlink()                                                               *
 *============================================================================*/

/**
 * @see fs_unlink().
 */
int vfs_unlink(int connection, const char *filename)
{
	/* Launch process. */
	if (fprocess_launch(connection) < 0)
		return (-EINVAL);

	return (fs_unlink(filename));
}

/*============================================================================*
 * vfs_read()                                                                 *
 *============================================================================*/

/**
 * @see fs_read().
 */
ssize_t vfs_read(int connection, int fd, void *buf, size_t n)
{
	/* Invalid file descriptor. */
	if (!WITHIN(fd, 0, NANVIX_OPEN_MAX))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* n too big */
	if (n > MINIX_BLOCK_SIZE)
		return (-EINVAL);

	/* Launch process. */
	if (fprocess_launch(connection) < 0)
		return (-EINVAL);

	return (fs_read(fd, buf, n));
}

/*============================================================================*
 * vfs_write()                                                                *
 *============================================================================*/

/**
 * @see fs_write().
 */
ssize_t vfs_write(int connection, int fd, void *buf, size_t n)
{
	/* Invalid file descriptor. */
	if (!WITHIN(fd, 0, NANVIX_OPEN_MAX))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* n too big */
	if (n > MINIX_BLOCK_SIZE)
		return (-EINVAL);

	/* Launch process. */
	if (fprocess_launch(connection) < 0)
		return (-EINVAL);

	return (fs_write(fd, buf, n));
}

/*============================================================================*
 * vfs_seek()                                                                 *
 *============================================================================*/

/**
 * @see vfs_seek().
 */
off_t vfs_seek(int connection, int fd, off_t offset, int whence)
{
	/* Invalid file descriptor. */
	if (!WITHIN(fd, 0, NANVIX_OPEN_MAX))
		return (-EINVAL);

	/* Launch process. */
	if (fprocess_launch(connection) < 0)
		return (-EINVAL);

	return (fs_lseek(fd, offset, whence));
}

/*============================================================================*
 * vfs_init()                                                                 *
 *============================================================================*/

/**
 * The vfs_init() function initializes the virtual file system.
 */
void vfs_init(void)
{
	fs_init();
	fprocess_init();
}

/*============================================================================*
 * vfs_shutdown()                                                             *
 *============================================================================*/

/**
 * The vfs_shutdown() function shutdowns the virtual file system.
 */
void vfs_shutdown(void)
{
	fs_shutdown();
}
