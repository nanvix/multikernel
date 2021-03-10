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
#define __NEED_LIMITS_PM
#define __NEED_NAME_SERVICE
#define __NEED_RESOURCE

#include <nanvix/limits/pm.h>
#include <nanvix/runtime/pm.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/**
 * @brief Input named mailbox.
 */
static int named_inboxes[NANVIX_PNAME_MAX];

/**
 * @brief Table of mailboxes.
 */
static struct named_mailbox
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource;        /**< Generic resource information. */

	int fd;                          /**< NoC connector.                */
	int owner;                       /**< Owner node.                   */
	char name[NANVIX_PROC_NAME_MAX]; /**< Name.                         */
} mailboxes[NANVIX_MAILBOX_MAX];

/**
 * @brief Pool of named mailboxes.
 */
static const struct resource_pool pool_mailboxes = {
	mailboxes, NANVIX_MAILBOX_MAX, sizeof(struct named_mailbox)
};

/**
 * @brief Input HAL mailbox.
 */
static int inboxes[NANVIX_PNAME_MAX];

/**
 *
 * @brief Is the inbox initialized ?
 */
static int initialized[NANVIX_PNAME_MAX] = { 0, };

/*============================================================================*
 * mailboxes_are_initialized()                                                *
 *============================================================================*/

/**
 * @brief Asserts whether or not the named mailbox facility was
 * initialized in the calling node.
 *
 * @returns One if the named mailbox facility was initialized, and zero
 * otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static int mailboxes_are_initialized(void)
{
	int nodenum;

	nodenum = knode_get_num();

	return (initialized[nodenum]);
}

/*============================================================================*
 * nanvix_mailbox_is_valid()                                                  *
 *============================================================================*/

/**
 * @brief Asserts whether or not a mailbox is valid.
 *
 * @param mbxid ID of the target mailbox.
 *
 * @returns One if the mailbox is valid, and zero otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static inline int nanvix_mailbox_is_valid(int mbxid)
{
	return ((mbxid >=0) && (mbxid < NANVIX_MAILBOX_MAX));
}

/*============================================================================*
 * nanvix_mailbox_create()                                                    *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_create(const char *name)
{
	int fd;      /* NoC connector. */
	int nodenum; /* NoC node.      */
	int mbxid;   /* ID of mailbix. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Check name length. */
	if (ustrlen(name) > KMAILBOX_MESSAGE_SIZE)
		return (-EINVAL);

	/* Runtime not initialized. */
	if ((fd = stdinbox_get()) < 0)
		return (-EAGAIN);

	/* Allocate mailbox. */
	if ((mbxid = resource_alloc(&pool_mailboxes)) < 0)
		return (-EAGAIN);

	nodenum = knode_get_num();

	/* Link name. */
	if (nanvix_name_link(nodenum, name) != 0)
		goto error0;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailboxes[mbxid].owner = nodenum;
	ustrcpy(mailboxes[mbxid].name, name);

	/* Initialize named inbox. */
	named_inboxes[core_get_id()] = mbxid;
	resource_set_rdonly(&mailboxes[mbxid].resource);

	return (mbxid);

error0:
	resource_free(&pool_mailboxes, mbxid);
	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_mailbox_create2()                                                   *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 *
 * @todo See if it's necessary to put the new mbxid in the named_inboxes table.
 */
int nanvix_mailbox_create2(const char *name, int port)
{
	int fd;      /* NoC connector. */
	int nodenum; /* NoC node.      */
	int mbxid;   /* ID of mailbox. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Check name length. */
	if (ustrlen(name) > KMAILBOX_MESSAGE_SIZE)
		return (-EINVAL);

	/* Check port number. */
	if (!WITHIN(port, NANVIX_GENERAL_PORTS_BASE, MAILBOX_PORT_NR))
		return (-EINVAL);

	nodenum = knode_get_num();

	/* Allocate mailbox. */
	if ((mbxid = resource_alloc(&pool_mailboxes)) < 0)
		return (-EAGAIN);

	/* Creates the underlying NoC connector. */
	if ((fd = kmailbox_create(nodenum, port)) < 0)
		goto error0;

	/* Link name. */
	if (nanvix_name_link(nodenum, name) != 0)
		goto error1;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailboxes[mbxid].owner = nodenum;
	ustrcpy(mailboxes[mbxid].name, name);

	resource_set_rdonly(&mailboxes[mbxid].resource);

	return (mbxid);

error1:
	if (kmailbox_unlink(fd) < 0)
		upanic("Could not clean an erroneus call to nanvix_mailbox_create2(). Aborting.");

error0:
	resource_free(&pool_mailboxes, mbxid);
	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_mailbox_open()                                                      *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_open(const char *name, int port)
{
	int fd;      /* NoC connector. */
	int nodenum; /* NoC node.      */
	int mbxid;   /* ID of mailbix. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Resolve name, */
	if ((nodenum = nanvix_name_lookup(name)) < 0)
		return (-EAGAIN);

	/* Allocate a mailbox. */
	if ((mbxid = resource_alloc(&pool_mailboxes)) < 0)
		return (-EAGAIN);

	/* Open underlying HW channel. */
	if ((fd = kmailbox_open(nodenum, port)) < 0)
		goto error0;

	/* Initialize mailbox. */
	mailboxes[mbxid].fd = fd;
	mailboxes[mbxid].owner = knode_get_num();
	resource_set_wronly(&mailboxes[mbxid].resource);

	return (mbxid);

error0:
	resource_free(&pool_mailboxes, mbxid);
	return (-EAGAIN);
}

/*============================================================================*
 * mailbox_read()                                                             *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_read(int mbxid, void *buf, size_t n)
{
	/* Invalid mailbox ID.*/
	if (!nanvix_mailbox_is_valid(mbxid))
		return (-EINVAL);

	/*  Bad mailbox. */
	if (!resource_is_used(&mailboxes[mbxid].resource))
		return (-EINVAL);

	/* Not the owner. */
	if (mailboxes[mbxid].owner != knode_get_num())
		return (-EPERM);

	/* Operation no supported. */
	if (!resource_is_rdonly(&mailboxes[mbxid].resource))
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Read. */
	if (kmailbox_read(mailboxes[mbxid].fd, buf, n) < 0)
		return (-EINVAL);

	return (0);
}

/*============================================================================*
 * mailbox_set_remote()                                                       *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_set_remote(int mbxid, int remote, int port)
{
	int ret;

	/* Invalid mailbox ID.*/
	if (!nanvix_mailbox_is_valid(mbxid))
		return (-EINVAL);

	/* Invalid nodenum. */
	if (!WITHIN(remote, 0, (MAILBOX_ANY_SOURCE + 1)))
		return (-EINVAL);

	/* Invalid port number. */
	if (!WITHIN(port, 0, (MAILBOX_ANY_PORT + 1)))
		return (-EINVAL);

	ret = kmailbox_set_remote(mailboxes[mbxid].fd, remote, port);

	return (ret);
}

/*============================================================================*
 * mailbox_write()                                                            *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_write(int mbxid, const void *buf, size_t n)
{
	/* Invalid mailbox ID.*/
	if (!nanvix_mailbox_is_valid(mbxid))
		return (-EINVAL);

	/* Bad mailbox. */
	if (!resource_is_used(&mailboxes[mbxid].resource))
		return (-EINVAL);

	/* Not the owner. */
	if (mailboxes[mbxid].owner != knode_get_num())
		return (-EPERM);

	/*  Invalid mailbox. */
	if (!resource_is_wronly(&mailboxes[mbxid].resource))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Write. */
	if (kmailbox_write(mailboxes[mbxid].fd, buf, n) < 0)
		return (-EINVAL);

	return (0);
}

/*============================================================================*
 * mailbox_close()                                                            *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_close(int mbxid)
{
	int r;		/* Return value. */

	/* Invalid mailbox ID.*/
	if (!nanvix_mailbox_is_valid(mbxid))
		return (-EINVAL);

	/* Bad mailbox. */
	if (!resource_is_used(&mailboxes[mbxid].resource))
		return (-EINVAL);

	/* Not the owner. */
	if (mailboxes[mbxid].owner != knode_get_num())
		return (-EPERM);

	/*  Invalid mailbox. */
	if (!resource_is_wronly(&mailboxes[mbxid].resource))
		return (-EINVAL);

	if ((r = kmailbox_close(mailboxes[mbxid].fd)) != 0)
		return (r);

	resource_free(&pool_mailboxes, mbxid);

	return (0);
}

/*============================================================================*
 * mailbox_unlink()                                                           *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_unlink(int mbxid)
{
	/* Invalid mailbox ID.*/
	if (!nanvix_mailbox_is_valid(mbxid))
		return (-EINVAL);

	/* Bad mailbox. */
	if (!resource_is_used(&mailboxes[mbxid].resource))
		return (-EINVAL);

	/* Not the owner. */
	if (mailboxes[mbxid].owner != knode_get_num())
		return (-EPERM);

	/*  Invalid mailbox. */
	if (!resource_is_rdonly(&mailboxes[mbxid].resource))
		return (-EINVAL);

	/* Unlink name. */
	if (nanvix_name_unlink(mailboxes[mbxid].name) != 0)
		return (-EAGAIN);

	resource_free(&pool_mailboxes, mbxid);

	return (0);
}

/*============================================================================*
 * nanvix_mailbox_get_inbox()                                                 *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int nanvix_mailbox_get_inbox(void)
{
	int local;

	/* Uninitialized named mailbox facility. */
	if (!mailboxes_are_initialized())
	{
		errno = EINVAL;
		return (-1);
	}

	local = knode_get_num();

	return (inboxes[local]);
}

/*============================================================================*
 * nanvix_mailbox_get_port()                                                  *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_mailbox_get_port(int mbxid)
{
	/* Invalid mailbox ID.*/
	if (!nanvix_mailbox_is_valid(mbxid))
		return (-EINVAL);

	/*  Bad mailbox. */
	if (!resource_is_used(&mailboxes[mbxid].resource))
		return (-EINVAL);

	return (kcomm_get_port(mailboxes[mbxid].fd, COMM_TYPE_MAILBOX));
}

/*============================================================================*
 * nanvix_mailbox_setup()                                                     *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_mailbox_setup(void)
{
	int local;

	local = knode_get_num();

	/* Nothing to do. */
	if (initialized[local])
		return (0);

	/* Initialize named mailboxes facility. */
	inboxes[local] = stdinbox_get();
	initialized[local] = 1;

	return (0);
}

/*============================================================================*
 * nanvix_mailbox_cleanup()                                                   *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_mailbox_cleanup(void)
{
	int local;

	/* Uninitialized named mailbox facility. */
	if (!mailboxes_are_initialized())
		return (-EINVAL);

	local = knode_get_num();

	initialized[local] = 0;

	return (0);
}
