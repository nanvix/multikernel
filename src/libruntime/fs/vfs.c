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
#define __NEED_FS_VFS_SERVER

#include <nanvix/runtime/runtime.h>
#include <nanvix/fs.h>
#include <nanvix/config.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

#ifdef __NANVIX_HAS_VFS_SERVER

/**
 * @brief Connection to VFSem Server.
 */
static struct
{
	int initialized; /**< Is the connection initialized? */
	int outbox;      /**< Output mailbox for requests.   */
	int outportal;   /**< Output portal for data.        */
} server = {
	.initialized = 0,
	.outbox = -1
};

/*============================================================================*
 * nanvix_vfs_stat()                                                          *
 *============================================================================*/

/**
 * @brief The do_nanvix_stat() function gets metadata about the file specified
 * by @p filename.
 */
static int do_nanvix_stat(const char *filename, struct nanvix_stat *restrict buf)
{
	struct vfs_message msg;

	/* Build message.*/
	message_header_build(&msg.header, VFS_STAT);
	ustrncpy(msg.op.stat.filename, filename, NANVIX_NAME_MAX);

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct vfs_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct vfs_message)
		) == sizeof(struct vfs_message)
	);

	/* Operation failed. */
	if (msg.header.opcode == VFS_FAIL)
		return (msg.op.ret.status);

	/* write answer to buffer */
	umemcpy(buf, &msg.op.stat.buf, sizeof(struct nanvix_stat));

	return (msg.op.ret.fd);
}

/**
 * @see do_nanvix_stat().
 *
 * @author Lucca Augusto
 */
int nanvix_vfs_stat(const char *filename, struct nanvix_stat *restrict buf)
{
	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid filename. */
	if (filename == NULL)
		return (-EINVAL);

	/* TODO: check for long filename. */
	if (ustrlen(filename) >= NANVIX_NAME_MAX)
		return (-ENAMETOOLONG);

	return (do_nanvix_stat(filename, buf));
}

/*============================================================================*
 * nanvix_vfs_open()                                                          *
 *============================================================================*/

/**
 * @brief The do_nanvix_vfs_open() function opens the file specified by
 * @p filename. The @p oflag parameter specifies the access mode, and
 * may be one of the following:
 *
 * - O_RDONLY opens a file for read-only.
 * - O_WRONLY opens a file for write-only.
 * - O_RDWR opens a file for both read and write.
 */
static int do_nanvix_vfs_open(const char *filename, int oflag, mode_t mode)
{
	struct vfs_message msg;

	/* Build message.*/
	message_header_build(&msg.header, VFS_OPEN);
	ustrncpy(msg.op.open.filename, filename, NANVIX_NAME_MAX);
	msg.op.open.oflag = oflag;
	msg.op.open.mode = mode;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct vfs_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct vfs_message)
		) == sizeof(struct vfs_message)
	);

	/* Operation failed. */
	if (msg.header.opcode == VFS_FAIL)
		return (msg.op.ret.status);

	return (msg.op.ret.fd);
}

/**
 * @see do_nanvix_vfs_open().
 *
 * @author Pedro Henrique Penna
 */
int nanvix_vfs_open(const char *filename, int oflag, mode_t mode)
{
	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid filename. */
	if (filename == NULL)
		return (-EINVAL);

	/* TODO: check for long filename. */
	if (ustrlen(filename) >= NANVIX_NAME_MAX)
		return (-ENAMETOOLONG);

	/* Invalid access mode.. */
	if (!ACCMODE_RDONLY(oflag) && !ACCMODE_WRONLY(oflag) && !ACCMODE_RDWR(oflag))
		return (-EINVAL);

	return (do_nanvix_vfs_open(filename, oflag, mode));
}

/*============================================================================*
 * nanvix_vfs_close()                                                         *
 *============================================================================*/

/**
 * The do_nanvix_vfs_close() function closes the file referred by the
 * file descriptor @p fd.
 *
 * @author Pedro Henrique Penna
 */
static int do_nanvix_vfs_close(int fd)
{
	struct vfs_message msg;

	/* Build message.*/
	message_header_build(&msg.header, VFS_CLOSE);
	msg.op.close.fd = fd;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct vfs_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct vfs_message)
		) == sizeof(struct vfs_message)
	);

	/* Operation failed. */
	if (msg.header.opcode == VFS_FAIL)
		return (msg.op.ret.status);

	return (0);
}

/**
 * @see do_nanvix_vfs_close().
 *
 * @author Pedro Henrique Penna
 */
int nanvix_vfs_close(int fd)
{
	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid file descriptor. */
	if (!NANVIX_VFS_FD_IS_VALID(fd))
		return (-EINVAL);

	return (do_nanvix_vfs_close(fd));
}

/*============================================================================*
 * nanvix_vfs_unlink()                                                        *
 *============================================================================*/

/**
 * The do_nanvix_vfs_unlink() function unlinks the file referred by the
 * file name @p filename.
 *
 * @author Lucca Augusto
 */
static int do_nanvix_vfs_unlink(const char *filename)
{
	struct vfs_message msg;

	/* Build message.*/
	message_header_build(&msg.header, VFS_UNLINK);
	ustrncpy(msg.op.unlink.filename, filename, NANVIX_NAME_MAX);

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct vfs_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct vfs_message)
		) == sizeof(struct vfs_message)
	);

	/* Operation failed. */
	if (msg.header.opcode == VFS_FAIL)
		return (msg.op.ret.status);

	return (0);
}

/**
 * @see do_nanvix_vfs_unlink().
 *
 * @author Lucca Augusto
 */
int nanvix_vfs_unlink(const char *filename)
{
	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid file name. */
	if (filename == NULL)
		return (-EINVAL);

	return (do_nanvix_vfs_unlink(filename));
}

/*============================================================================*
 * nanvix_vfs_seek()                                                          *
 *============================================================================*/

/**
 * The do_nanvix_vfs_seek() function repositions the read/write pointer
 * of the file referred by the file descriptor @p fd. The new position
 * for the read/write pointer is computed according to the value of @p
 * whence:
 *
 * - SEEK_SET sets the read/write pointer is set to @p offset.
 * - SEEK_CUR increases the read/write pointer by @p offset.
 * - SEEK_END sets the read/write pointer to the end of the file plus @p offset
 *
 * @author Pedro Henrique Penna
 */
static off_t do_nanvix_vfs_seek(int fd, off_t offset, int whence)
{
	struct vfs_message msg;

	/* Build message.*/
	message_header_build(&msg.header, VFS_SEEK);
	msg.op.seek.fd = fd;
	msg.op.seek.offset = offset;
	msg.op.seek.whence = whence;

	/* Send operation. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct vfs_message)
		) == 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct vfs_message)
		) == sizeof(struct vfs_message)
	);

	/* Operation failed. */
	if (msg.header.opcode == VFS_FAIL)
		return (msg.op.ret.status);

	return (msg.op.ret.offset);
}

/**
 * @see do_nanvix_vfs_seek().
 *
 * @author Pedro Henrique Penna
 */
off_t nanvix_vfs_seek(int fd, off_t offset, int whence)
{
	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid file descriptor. */
	if (!NANVIX_VFS_FD_IS_VALID(fd))
		return (-EINVAL);

	/* Invalid whence. */
	if ((whence != SEEK_SET) && (whence != SEEK_CUR) && (whence != SEEK_END))
		return (-EINVAL);

	return (do_nanvix_vfs_seek(fd, offset, whence));
}

/*============================================================================*
 * nanvix_vfs_read()                                                          *
 *============================================================================*/

/**
 * The do_nanvix_vfs_read() function reads @p n bytes from the file
 * referred by the file descriptor @p fd into the buffer pointed to by
 * @p buf.
 *
 * @author Pedro Henrique Penna
 *
 * @todo TODO: tile data transfers
 */
static ssize_t do_nanvix_vfs_read(int fd, void *buf, size_t n)
{
	struct vfs_message msg;

	/* Nothing to do. */
	if (n == 0)
		return (0);

	/* Build message.*/
	message_header_build(&msg.header, VFS_READ);
	msg.op.read.fd = fd;
	msg.op.read.n = n;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct vfs_message)
		) == 0
	);

	/* Wait acknowledge. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct vfs_message)
		) == sizeof(struct vfs_message)
	);
	uassert(msg.header.opcode == VFS_ACK);

	/* Receive data. */
	uassert(
		kportal_allow(
			stdinportal_get(),
			VFS_SERVER_NODE,
			msg.header.portal_port
		) == 0
	);
	uassert(
		kportal_read(
			stdinportal_get(),
			buf,
			n
		) >= 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct vfs_message)
		) == sizeof(struct vfs_message)
	);

	/* Operation failed. */
	if (msg.header.opcode == VFS_FAIL)
		return (msg.op.ret.status);

	return (msg.op.ret.count);
}

/**
 * @see do_nanvix_vfs_read().
 *
 * @author Pedro Henrique Penna
 */
ssize_t nanvix_vfs_read(int fd, void *buf, size_t n)
{
	char *pbase; /* Base Address of Buffer */
	char *pend;  /* End Address of Buffer  */

	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid file descriptor. */
	if (!NANVIX_VFS_FD_IS_VALID(fd))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid read size. */
	if (n > NANVIX_MAX_FILE_SIZE)
		return (-EFBIG);

	/* Read in chunks. */
	pbase = buf; pend = ((char *) buf) + n;
	for (char *p = pbase; p < pend; p += NANVIX_FS_BLOCK_SIZE)
	{
		size_t count;
		ssize_t nread;

		count = ((pend - p) < NANVIX_FS_BLOCK_SIZE) ?
			(size_t)(pend - p) : NANVIX_FS_BLOCK_SIZE;

		if ((nread = do_nanvix_vfs_read(fd, p, count)) < 0)
			return (nread);
	}

	return ((size_t)(pend - pbase));
}

/*============================================================================*
 * nanvix_vfs_write()                                                         *
 *============================================================================*/

/**
 * The do_nanvix_vfs_write() function writes @p n bytes from the buffer
 * pointed to by @p buf to the filereferred by the file descriptor @p
 * fd.
 *
 * @author Pedro Henrique Penna
 *
 * @todo TODO: tile data transfers
 */
ssize_t do_nanvix_vfs_write(int fd, const void *buf, size_t n)
{
	struct vfs_message msg;

	/* Nothing to do. */
	if (n == 0)
		return (0);

	/* Build message.*/
	message_header_build2(
		&msg.header,
		VFS_WRITE,
		nanvix_portal_get_port(server.outportal)
	);
	msg.op.write.fd = fd;
	msg.op.write.n = n;

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg,
			sizeof(struct vfs_message)
		) == 0
	);

	/* Send data. */
	uassert(
		nanvix_portal_write(
			server.outportal,
			buf,
			n
		) >= 0
	);

	/* Receive reply. */
	uassert(
		kmailbox_read(
			stdinbox_get(),
			&msg,
			sizeof(struct vfs_message)
		) == sizeof(struct vfs_message)
	);

	/* Operation failed. */
	if (msg.header.opcode == VFS_FAIL)
		return (msg.op.ret.status);

	return (msg.op.ret.count);
}

/**
 * @see do_nanvix_vfs_write().
 *
 * @author Pedro Henrique Penna
 */
ssize_t nanvix_vfs_write(int fd, const void *buf, size_t n)
{
	const char *pbase; /* Base Address of Buffer */
	const char *pend;  /* End Address of Buffer  */

	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Invalid file descriptor. */
	if (!NANVIX_VFS_FD_IS_VALID(fd))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid write size. */
	if (n > NANVIX_MAX_FILE_SIZE)
		return (-EFBIG);

	/* Read in chunks. */
	pbase = buf; pend = ((char *) buf) + n;
	for (const char *p = pbase; p < pend; p += NANVIX_FS_BLOCK_SIZE)
	{
		size_t count;
		ssize_t nwrite;

		count = ((pend - p) < NANVIX_FS_BLOCK_SIZE) ?
			(size_t)(pend - p) : NANVIX_FS_BLOCK_SIZE;

		if ((nwrite = do_nanvix_vfs_write(fd, p, count)) < 0)
			return (nwrite);
	}

	return ((size_t)(pend - pbase));
}

#endif

/*============================================================================*
 * nanvix_vfs_shutdown()                                                      *
 *============================================================================*/

/**
 * The nanvix_vfs_shutdown() function issues the shutdown signal to the
 * VFS server, thus causing the server to terminate.
 */
int nanvix_vfs_shutdown(void)
{
#ifdef __NANVIX_HAS_VFS_SERVER

	struct vfs_message msg;

	/* Invalid server ID. */
	if (!server.initialized)
		return (-EAGAIN);

	/* Build operation header. */
	message_header_build(&msg.header, VFS_EXIT);

	/* Send operation header. */
	uassert(
		nanvix_mailbox_write(
			server.outbox,
			&msg, sizeof(struct vfs_message)
		) == 0
	);

#endif

	return (0);
}

/*============================================================================*
 * __nanvix_vfs_setup()                                                       *
 *============================================================================*/

/**
 * The __nanvhx_vfs_setup() function initializes the local daemon for
 * the VFS service.
 */
int __nanvix_vfs_setup(void)
{
#ifdef __NANVIX_HAS_VFS_SERVER

	/* Nothing to do.  */
	if (server.initialized)
		return (0);

	/* Open output mailbox */
	if ((server.outbox = nanvix_mailbox_open(VFS_SERVER_NAME, VFS_SERVER_PORT_NUM)) < 0)
	{
		uprintf("[nanvix][vfs] cannot open outbox to server");
		return (server.outbox);
	}

	/* Open underlying IPC connectors. */
	if ((server.outportal = nanvix_portal_open(VFS_SERVER_NAME, VFS_SERVER_PORT_NUM)) < 0)
	{
		uprintf("[nanvix][vfs] cannot open outportal to server");
		return (server.outportal);
	}

	server.initialized = true;
	uprintf("[nanvix][vfs] connection with server established");

#endif

	return (0);
}

/*============================================================================*
 * __nanvix_vfs_cleanup()                                                     *
 *============================================================================*/

/**
 * The __nanvix_vfs_cleanup() function shutdowns the local daemon for
 * the VFS service.
 */
int __nanvix_vfs_cleanup(void)
{

#ifdef __NANVIX_HAS_VFS_SERVER

	int ret;

	/* Nothing to do. */
	if (!server.initialized)
		return (0);

	/* Close output mailbox. */
	if ((ret = nanvix_mailbox_close(server.outbox)) < 0)
	{
		uprintf("[nanvix][vfs] cannot close outbox to server");
		return (ret);
	}

	/* Close underlying IPC connectors. */
	if (nanvix_portal_close(server.outportal) < 0)
	{
		uprintf("[nanvix][vfs] cannot close outportal to server");
		return (-EAGAIN);
	}

	server.initialized = 0;

#endif

	return (0);
}
