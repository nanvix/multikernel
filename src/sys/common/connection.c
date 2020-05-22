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

#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

/**
 * @brief Connections
 */
static struct
{
	pid_t remote;  /**< PID.                  */
	int count;     /**< Number of Connections */
} connections[NANVIX_CONNECTIONS_MAX];

/*============================================================================*
 * connect()                                                                  *
 *============================================================================*/

/**
 * The lookup() function searches in the table of active connnections
 * for the connection in which the process @p remote is hooked up.
 */
int lookup(pid_t remote)
{
	/* Invalid PID. */
	if (remote < 0)
		return (-EINVAL);

	/* Lookup connection. */
	for (int i = 0; i < NANVIX_CONNECTIONS_MAX; i++)
	{
		/* Found. */
		if (connections[i].remote == remote)
			return (i);
	}
	
	return (-ENOENT);
}

/*============================================================================*
 * connect()                                                                  *
 *============================================================================*/

/**
 * The connect() function connects the remote client @p remote to the
 * running server.
 */
int connect(pid_t remote)
{
	int i;

	/* Invalid PID. */
	if (remote < 0)
		return (-EINVAL);

	/* Registered remote? */
	if ((i = lookup(remote)) < 0)
	{
		/* Looks for an empty slot in the table of connections. */
		for (i = 0; i < NANVIX_CONNECTIONS_MAX; i++)
		{
			/* Found. */
			if (connections[i].remote < 0)
			{
				connections[i].remote = remote;
				connections[i].count = 0;
				goto out;
			}
		}

		return (-EAGAIN);
	}

out:

	connections[i].count++;
	
	return (0);
}

/*============================================================================*
 * disconnect()                                                               *
 *============================================================================*/

/**
 * The disconnect() function disconnects the remote client @p remote to
 * the running server.
 */
int disconnect(pid_t remote)
{
	int i;

	/* Invalid PID. */
	if (remote < 0)
		return (-EINVAL);

	/* Registered remote? */
	if ((i = lookup(remote)) < 0)
		return (-ENOENT);

	/* Unlink remote. */
	if (connections[i].count-- == 1)
		connections[i].remote = -1;
	
	return (0);
}

/*============================================================================*
 * connect()                                                                  *
 *============================================================================*/

/**
 * The get_connections() returns the connections that are established
 * with the running server. Connections are place in the buffer pointed
 * to by @p buf.
 */
int get_connections(pid_t *buf)
{
	int count;

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Traverse table of connections. */
	count = 0;
	for (int i = 0; i < NANVIX_CONNECTIONS_MAX; i++)
	{
		/* Skip invalid connections. */
		if (connections[i].remote < 0)
			continue;

		buf[count++] = connections[i].remote;
	}
	
	return (count);
}

/*============================================================================*
 * connections_setup()                                                        *
 *============================================================================*/

/**
 * The connections_setup() initializes the table of connections.
 */
void connections_setup(void)
{
	for (int i = 0; i < NANVIX_CONNECTIONS_MAX; i++)
	{
		connections[i].remote = -1;
		connections[i].count = 0;
	}
}
