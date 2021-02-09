/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Must come first. */
#define __NEED_LIMITS_PM
#define __NEED_NAME_SERVICE
#define __NEED_RESOURCE

#include <nanvix/limits/pm.h>
#include <nanvix/runtime/pm.h>
#include <nanvix/sys/thread.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/**
 * @brief Table of portals.
 */
static struct named_portal
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource;        /**< Generic resource information. */

	int portalid;                    /**< Underlying unnamed portal.    */
	int owner;                       /**< Owner node.                   */
	char name[NANVIX_PROC_NAME_MAX]; /**< Name.                         */
} portals[NANVIX_PORTAL_MAX];

/**
 * @brief Pool of named portals.
 */
static const struct resource_pool pool_portals = {
	portals, NANVIX_PORTAL_MAX, sizeof(struct named_portal)
};

/*============================================================================*
 * nanvix_portal_is_valid()                                                   *
 *============================================================================*/

/**
 * @brief Asserts whether or not a portal is valid.
 *
 * @param id ID of the target portal.
 *
 * @returns One if the portal is valid, and zero otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static inline int nanvix_portal_is_valid(int id)
{
	return ((id >= 0) && (id < NANVIX_PORTAL_MAX));
}

/*============================================================================*
 * __nanvix_portal_setup()                                                    *
 *============================================================================*/

/**
 * @brief input portals.
 */
static int inportals[NANVIX_PNAME_MAX];

/**
 * @brief Is the named portals facility initialized?
 */
static int initialized[NANVIX_PNAME_MAX] = { 0 , };

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_portal_setup(void)
{
	int local;

	local = knode_get_num();

	/* Nothing to do. */
	if (initialized[local])
		return (0);

	/* Initialize named portals facility. */
	inportals[local] = stdinportal_get();
	initialized[local] = 1;

	return (0);
}

/*============================================================================*
 * portals_are_initialized()                                                  *
 *============================================================================*/

/**
 * @brief Asserts whether or not the named portal facility was
 * initialized in the calling node.
 *
 * @returns One if the named portal facility was initialized, and zero
 * otherwise.
 *
 * @note This function is @b NOT thread safe.
 */
static int portals_are_initialized(void)
{
	int nodenum;

	nodenum = knode_get_num();

	return (initialized[nodenum]);
}

/*============================================================================*
 * get_inportal()                                                             *
 *============================================================================*/

/**
 * @brief Returns the underlying unnamed input portal.
 */
int get_inportal(void)
{
	int local;

	/* Uninitialized named portal facility. */
	if (!portals_are_initialized())
		return (-EINVAL);

	local = knode_get_num();

	return (inportals[local]);
}

/*============================================================================*
 * __nanvix_portal_cleanup()                                                  *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int __nanvix_portal_cleanup(void)
{
	int local;

	/* Uninitialized named portal facility. */
	if (!portals_are_initialized())
		return (-EINVAL);

	local = knode_get_num();

	initialized[local] = 0;

	return (0);
}

/*============================================================================*
 * nanvix_portal_create()                                                     *
 *============================================================================*/

/**
 * @brief Creates a portal.
 *
 * @param name Portal name.
 *
 * @returns Upon successful completion, the ID of the new portal is
 * returned. Upon failure, a negative error code is returned instead.
 */
int nanvix_portal_create(const char *name)
{
	int id;       /* Portal ID.                 */
	int nodenum;  /* NoC node.                  */
	int portalid; /* Underlying unnamed portal. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Check name length. */
	if (ustrlen(name) > KMAILBOX_MESSAGE_SIZE)
		return (-EINVAL);

	/* Runtime not initialized. */
	if ((portalid = stdinportal_get()) < 0)
		return (-EAGAIN);

	/* Allocate portal. */
	if ((id = resource_alloc(&pool_portals)) < 0)
		return (-EAGAIN);

	nodenum = knode_get_num();

	/* Link name. */
	if (nanvix_name_link(nodenum, name) != 0)
		goto error0;

	/* Initialize portal. */
	portals[id].portalid = portalid;
	portals[id].owner = nodenum;
	ustrcpy(portals[id].name, name);

	inportals[core_get_id()] = id;
	resource_set_rdonly(&portals[id].resource);

	return (id);

error0:
	resource_free(&pool_portals, id);
	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_portal_create2()                                                    *
 *============================================================================*/

/**
 * @brief Creates a portal specifying the input port.
 *
 * @param name Portal name.
 * @param port Local port.
 *
 * @returns Upon successful completion, the ID of the new portal is
 * returned. Upon failure, a negative error code is returned instead.
 *
 * @note This function should be called only for creating portals with
 * ports specified in the special range that is not related with the
 * standard ones.
 */
int nanvix_portal_create2(const char *name, int port)
{
	int fd;       /* NoC connector. */
	int nodenum;  /* NoC node.      */
	int portalid; /* Portal ID.     */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Check name length. */
	if (ustrlen(name) > KMAILBOX_MESSAGE_SIZE)
		return (-EINVAL);

	/* Check port number. */
	if (!WITHIN(port, NANVIX_GENERAL_PORTS_BASE, KPORTAL_PORT_NR))
		return (-EINVAL);

	nodenum = knode_get_num();

	/* Allocate portal. */
	if ((portalid = resource_alloc(&pool_portals)) < 0)
		return (-EAGAIN);

	/* Creates the underlying NoC connector. */
	if ((fd = kportal_create(nodenum, port)) < 0)
		goto error0;

	/* Link name. */
	if (nanvix_name_link(nodenum, name) != 0)
		goto error1;

	/* Initialize portal. */
	portals[portalid].portalid = fd;
	portals[portalid].owner = nodenum;
	ustrcpy(portals[portalid].name, name);

	resource_set_rdonly(&portals[portalid].resource);

	return (portalid);

error1:
	if (kportal_unlink(fd) < 0)
		upanic("Could not clean an erroneus call to nanvix_portal_create2(). Aborting.");

error0:
	resource_free(&pool_portals, portalid);
	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_portal_allow()                                                      *
 *============================================================================*/

/**
 * @brief Enables read operations on a portal.
 *
 * @param id	   ID of the target portal.
 * @param nodenum  Target node.
 *
 * @returns Upons successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @note This function is NOT thread-safe.
 */
int nanvix_portal_allow(int id, int nodenum)
{
	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(id))
		return (-EINVAL);

	/*  Bad portal. */
	if (!resource_is_used(&portals[id].resource))
		return (-EINVAL);

	/* Operation no supported. */
	if (!resource_is_rdonly(&portals[id].resource))
		return (-ENOTSUP);

	/* Not the owner. */
	if (portals[id].owner != knode_get_num())
		return (-EINVAL);

	return (kportal_allow(portals[id].portalid, nodenum, kthread_self()));
}

/*============================================================================*
 * nanvix_portal_allow2()                                                     *
 *============================================================================*/

/**
 * @brief Enables read operations on a portal specifying remote port.
 *
 * @param id	   ID of the target portal.
 * @param nodenum  Target node.
 * @param port     Remote port.
 *
 * @returns Upons successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 *
 * @note This function is NOT thread-safe.
 */
PUBLIC int nanvix_portal_allow2(int id, int nodenum, int port)
{
	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(id))
		return (-EINVAL);

	/*  Bad portal. */
	if (!resource_is_used(&portals[id].resource))
		return (-EINVAL);

	/* Operation no supported. */
	if (!resource_is_rdonly(&portals[id].resource))
		return (-ENOTSUP);

	/* Not the owner. */
	if (portals[id].owner != knode_get_num())
		return (-EINVAL);

	return (kportal_allow(portals[id].portalid, nodenum, port));
}

/*============================================================================*
 * nanvix_portal_open()                                                       *
 *============================================================================*/

/**
 * @brief Opens a portal.
 *
 * @param name Portal name.
 *
 * @returns Upon successful completion, the ID of the target portal is
 * returned. Upon failure, a negative error code is returned instead.
 */
int nanvix_portal_open(const char *name, int port)
{
	int id;	   /* Portal ID.				 */
	int nodenum;  /* NoC node.				  */
	int portalid; /* Underlying unnamed portal. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Resolve name. */
	if ((nodenum = nanvix_name_lookup(name)) < 0)
		return (-EAGAIN);

	/* Allocate a portal. */
	if ((id = resource_alloc(&pool_portals)) < 0)
		return (-EAGAIN);

	/* Open underlying unnamed portal. */
	if ((portalid = kportal_open(knode_get_num(), nodenum, port)) < 0)
		goto error0;

	/* Initialize portal. */
	portals[id].portalid = portalid;
	portals[id].owner = knode_get_num();
	resource_set_wronly(&portals[id].resource);

	return (id);

error0:
	resource_free(&pool_portals, id);
	return (-EAGAIN);
}

/*============================================================================*
 * nanvix_portal_read()                                                       *
 *============================================================================*/

/**
 * @brief Reads data from a portal.
 *
 * @param id  ID of the target portal.
 * @param buf Location from where data should be written.
 * @param n   Number of bytes to write.
 *
 * @returns See kportal_aread().
 */
int nanvix_portal_read(int id, void *buf, size_t n)
{
	int ret;

	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(id))
		return (-EINVAL);

	/*  Bad portal. */
	if (!resource_is_used(&portals[id].resource))
		return (-EINVAL);

	/* Operation no supported. */
	if (!resource_is_rdonly(&portals[id].resource))
		return (-ENOTSUP);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid buffer size. */
	if (n < 1)
		return (-EINVAL);

	/* Not the owner. */
	if (portals[id].owner != knode_get_num())
		return (-EINVAL);

	ret = kportal_read(portals[id].portalid, buf, n);

	return (ret);
}

/*============================================================================*
 * nanvix_portal_write()                                                      *
 *============================================================================*/

/**
 * @brief Writes data to a portal.
 *
 * @param id  ID of the target portal.
 * @param buf Location from where data should be read.
 * @param n   Number of bytes to write.
 *
 * @returns See kportal_awrite();
 */
int nanvix_portal_write(int id, const void *buf, size_t n)
{
	int ret;

	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(id))
		return (-EINVAL);

	/* Bad portal. */
	if (!resource_is_used(&portals[id].resource))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!resource_is_wronly(&portals[id].resource))
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid buffer. */
	if (n < 1)
		return (-EINVAL);

	/* Not the owner. */
	if (portals[id].owner != knode_get_num())
		return (-EINVAL);

	ret = kportal_write(portals[id].portalid, buf, n);

	return (ret);
}

/*============================================================================*
 * nanvix_portal_close()                                                      *
 *============================================================================*/

/**
 * @brief Closes a portal.
 *
 * @param id ID of the target portal.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int nanvix_portal_close(int id)
{
	int err;

	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(id))
		return (-EINVAL);

	/* Bad portal. */
	if (!resource_is_used(&portals[id].resource))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!resource_is_wronly(&portals[id].resource))
		return (-EINVAL);

	/* Not the owner. */
	if (portals[id].owner != knode_get_num())
		return (-EINVAL);

	/* Close underlying unnamed portal. */
	if ((err = kportal_close(portals[id].portalid)) != 0)
		return (err);

	resource_free(&pool_portals, id);

	return (0);
}

/*============================================================================*
 * nanvix_portal_unlink()                                                     *
 *============================================================================*/

/**
 * @brief Destroys a portal.
 *
 * @param id ID of the target portal.
 *
 * @returns Upon successful completion zero, is returned. Upon
 * failure, a negative error code is returned instead.
 *
 * @note This function is @b NOT thread safe.
 */
int nanvix_portal_unlink(int id)
{
	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(id))
		return (-EINVAL);

	/* Bad portal. */
	if (!resource_is_used(&portals[id].resource))
		return (-EINVAL);

	/*  Invalid portal. */
	if (!resource_is_rdonly(&portals[id].resource))
		return (-EINVAL);

	/* Unlink name. */
	if (nanvix_name_unlink(portals[id].name) != 0)
		return (-EAGAIN);

	/* Not the owner. */
	if (portals[id].owner != knode_get_num())
		return (-EINVAL);

	/*
	 * Underlying unnamed mailbox is
	 * destroyed when unloading the runtime system
	 */

	resource_free(&pool_portals, id);

	return (0);
}

/*============================================================================*
 * nanvix_portal_get_port()                                                   *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_portal_get_port(int portalid)
{
	/* Invalid portal ID.*/
	if (!nanvix_portal_is_valid(portalid))
		return (-EINVAL);

	/*  Bad portal. */
	if (!resource_is_used(&portals[portalid].resource))
		return (-EINVAL);

	/*  Bad portal. */
	if (!resource_is_wronly(&portals[portalid].resource))
		return (-EINVAL);

	return (kcomm_get_port(portals[portalid].portalid, COMM_TYPE_PORTAL));
}
