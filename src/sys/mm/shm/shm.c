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
#define __SHM_SERVER
#define __NEED_RESOURCE
#define __NEED_MM_RMEM_STUB

#include <nanvix/runtime/mm.h>
#include <nanvix/servers/shm.h>
#include <nanvix/sys/noc.h>
#include <nanvix/ulib.h>
#include <posix/sys/stat.h>
#include <posix/errno.h>
#include <posix/fcntl.h>

/**
 * @brief Table of shared memory regions.
 */
static struct region
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource;        /**< Generic resource information.  */

	char name[NANVIX_SHM_NAME_MAX];  /**< Shared memory region name.     */
	pid_t owner;                     /**< ID of owner process.           */
	int refcount;                    /**< Number of references.          */
	mode_t mode;                     /**< Access permissions.            */
	size_t size;                     /**< Size (in bytes).               */
	rpage_t page;                    /**< Remote page                    */
} regions[NANVIX_SHM_MAX];

/**
 * @brief Pool of shared memory regions.
 */
static struct resource_pool pool = {
	regions, NANVIX_SHM_MAX, sizeof(struct region)
};

/*============================================================================*
 * shm_is_remove()                                                            *
 *============================================================================*/

/**
 * @brief Asserts if a shared memory region is to be removed.
 *
 * @param shmid ID of the target shared memory region.
 *
 * @returns Non-zero if the shared memory region is marked to be
 * removed, and zero otherwise.
 */
static inline int shm_is_remove(int shmid)
{
	return (!resource_is_busy(&regions[shmid].resource));
}

/*============================================================================*
 * shm_is_owner()                                                             *
 *============================================================================*/

/**
 * @brief Asserts if a process owns a shared memory region.
 *
 * @param shmid Target shared memory region.
 * @param process  Target process.
 *
 * @returns Non-zero if the target shared memory region is owned by
 * the target process, and zero otherwise.
 */
static inline int shm_is_owner(int shmid, pid_t proc)
{
	return (regions[shmid].owner == proc);
}

/*============================================================================*
 * shm_is_mapped()                                                            *
 *============================================================================*/

/**
 * @brief Asserts if a shared memory region is mapped.
 *
 * @param shmid Target shared memory region.
 *
 * @returns Non-zero if the target shared memory region is mapped,
 * and zero otherwise.
 */
static inline int shm_is_mapped(int shmid)
{
	return (resource_is_mapped(&regions[shmid].resource));
}

/*============================================================================*
 * shm_is_readable()                                                          *
 *============================================================================*/

/**
 * @brief Asserts if a shared memory region has read permissions.
 *
 * @param shmid Target shared memory region.
 *
 * @returns Non-zero if the target shared memory region is readable,
 * and zero otherwise.
 */
static inline int shm_is_readable(int shmid)
{
	return (resource_is_readable(&regions[shmid].resource));
}

/*============================================================================*
 * shm_is_writable()                                                          *
 *============================================================================*/

/**
 * @brief Asserts if a shared memory region has write permissions.
 *
 * @param shmid Target shared memory region.
 *
 * @returns Non-zero if the target shared memory region is writable,
 * and zero otherwise.
 */
static inline int shm_is_writable(int shmid)
{
	return (resource_is_writable(&regions[shmid].resource));
}

/*============================================================================*
 * shm_get_size()                                                             *
 *============================================================================*/

/**
 * @brief Gets the size (in bytes) of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 *
 * @returns The size (in bytes) of the target shared memory region.
 */
static inline size_t shm_get_size(int shmid)
{
	return (regions[shmid].size);
}

/*============================================================================*
 * shm_set_remove()                                                           *
 *============================================================================*/

/**
 * @brief Marks a shared memory region to be removed.
 *
 * @param shmid ID of the target shared memory region.
 */
static inline void shm_set_remove(int shmid)
{
	resource_set_notbusy(&regions[shmid].resource);
}

/*============================================================================*
 * shm_set_owner()                                                            *
 *============================================================================*/

/**
 * @brief Sets the the owner of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 * @param owner ID of the onwer process.
 */
static inline void shm_set_owner(int shmid, pid_t owner)
{
	regions[shmid].owner = owner;
}

/*============================================================================*
 * shm_set_perm()                                                             *
 *============================================================================*/

/**
 * @brief Sets the access permissions of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 */
static inline void shm_set_perm(int shmid, mode_t mode)
{
	regions[shmid].mode = mode;

	if (regions[shmid].mode & S_IWUSR)
		resource_set_rdwr(&regions[shmid].resource);
}

/*============================================================================*
 * shm_set_name()                                                             *
 *============================================================================*/

/**
 * @brief Sets the name of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 * @param name  Name for the target shared memory region.
 */
static inline void shm_set_name(int shmid, const char *name)
{
	ustrcpy(regions[shmid].name, name);
}

/*============================================================================*
 * shm_set_size()                                                             *
 *============================================================================*/

/**
 * @brief Sets the size (in bytes) of a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 * @param size  Size of the target shared memoy region.
 */
static inline void shm_set_size(int shmid, size_t size)
{
	/* Allocate page. */
	if ((size > 0) && (regions[shmid].page == RMEM_NULL))
		uassert((regions[shmid].page = nanvix_rmem_alloc()) != RMEM_NULL);

	/* Release page. */
	else if ((size == 0) && (regions[shmid].page != RMEM_NULL))
	{
		uassert(nanvix_rmem_free(regions[shmid].page) == 0);
		regions[shmid].page = RMEM_NULL;
	}

	regions[shmid].size = size;
}

/*============================================================================*
 * shm_alloc()                                                                *
 *============================================================================*/

/**
 * @brief Allocates a shared memory region.
 *
 * @return Upon successful completion, the ID of the newly allocated
 * shared memory region is returned. Upon failure, -1 is returned instead.
 */
int shm_alloc(void)
{
	int shmid;

	if ((shmid = resource_alloc(&pool)) < 0)
		return (-1);

	regions[shmid].refcount = 1;
	resource_set_busy(&regions[shmid].resource);
	resource_set_rdonly(&regions[shmid].resource);

	return (shmid);
}

/*============================================================================*
 * shm_free()                                                                 *
 *============================================================================*/

/**
 * @brief Free a shared memory region.
 */
static void shm_free(int shmid)
{
	resource_free(&pool, shmid);
}

/*============================================================================*
 * shm_get()                                                                  *
 *============================================================================*/

/**
 * @brief Gets a shared memory region.
 *
 * @param name Name of the target shared memory region.
 *
 * @return If the target shared memory region is found, its ID is
 * returned. Otherwise, -1 is returned instead.
 */
int shm_get(const char *name)
{
	for (int i = 0; i < NANVIX_SHM_MAX; i++)
	{
		/* Skip invalid shared memory regions. */
		if (!resource_is_used(&regions[i].resource))
			continue;

		/* Found.*/
		if (!ustrcmp(regions[i].name, name))
		{
			regions[i].refcount++;
			return (i);
		}
	}

	return (-1);
}

/*============================================================================*
 * shm_put()                                                                  *
 *============================================================================*/

/**
 * @brief Releases a shared memory region.
 *
 * @param shmid ID of the target shared memory region.
 */
static int shm_put(int shmid)
{
	if (regions[shmid].refcount == 0)
		return (-EINVAL);

	regions[shmid].refcount--;

	/* Unlink the shared memory region if no process is using it anymore. */
	if ((regions[shmid].refcount == 0) && (shm_is_remove(shmid)))
	{
		/* Release underlying page. */
		if (regions[shmid].page != RMEM_NULL)
		{
			uassert(nanvix_rmem_free(regions[shmid].page) == 0);
			regions[shmid].page = RMEM_NULL;
		}

		shm_free(shmid);
	}

	return (0);
}

/*============================================================================*
 * __do_shm_ftruncate()                                                       *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __do_shm_ftruncate(
	rpage_t *page,
	pid_t proc,
	int shmid,
	off_t size
)
{
	shm_debug("ftruncate proc=%d shmid=%d", proc, shmid);

	/* Invalid shared memory region. */
	if (!nanvix_shm_is_valid(shmid))
		return (-EINVAL);

	/* Bad shared memory. */
	if (!resource_is_used(&regions[shmid].resource))
		return (-EINVAL);

	/* Invalid size. */
	if (size < 0)
		return (-EINVAL);

	/* Size too big. */
	if (size > NANVIX_SHM_SIZE_MAX)
		return (-EFBIG);

	/*
	 * TODO check if this process can truncate the shared memory region.
	 */
	((void) proc);

	/* Cannot write. */
	if (!shm_is_writable(shmid))
		return (-EACCES);

	/* Already mapped. */
	if (shm_is_mapped(shmid))
		return (-EBUSY);

	shm_set_size(shmid, size);

	*page = regions[shmid].page;

	return (0);
}

/*============================================================================*
 * do_open()                                                                  *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __do_shm_open(
	rpage_t *page,
	pid_t proc,
	const char *name,
	int oflags
)
{
	int shmid;

	shm_debug("open proc=%d name=%s", proc, name);

	/*
	 * TODO: check if process ID is valid.
	 */

	/* Invalid name. */
	if (nanvix_shm_name_is_invalid(name))
		return (-EINVAL);

	/* Invalid opening flags. */
	if (oflags & ~(O_TRUNC | O_ACCMODE))
		return (-ENOTSUP);

	/* Get shared memory. */
	if ((shmid = shm_get(name)) < 0)
		return (-EINVAL);

	/*
	 * TODO: check acess permissions.
	 */
	((void) proc);

	/* Shared memory region shall be removed soon. */
	if (shm_is_remove(shmid))
	{
		shm_put(shmid);
		return (-EAGAIN);
	}

	/* Truncate. */
	if (oflags & O_TRUNC)
	{
		int ret;

		/* Cannot write. */
		if (!(oflags & (O_RDWR | O_WRONLY)))
		{
			shm_put(shmid);
			return (-EACCES);
		}

		/* Truncate. */
		if ((ret = __do_shm_ftruncate(page, proc, shmid, 0)) < 0)
		{
			shm_put(shmid);
			return (-EAGAIN);
		}
	}

	*page = regions[shmid].page;

	return (shmid);
}

/*============================================================================*
 * __do_create()                                                              *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __do_shm_create(
	rpage_t *page,
	pid_t proc,
	const char *name,
	int oflags,
	mode_t mode
)
{
	int shmid;

	shm_debug("create proc=%d name=%s oflags=%x mode=%x", proc, name, oflags, mode);

	/*
	 * TODO: check if process ID is valid.
	 */

	/* Invalid name. */
	if (nanvix_shm_name_is_invalid(name))
		return (-EINVAL);

	/* Invalid opening flags. */
	if (oflags & ~(O_CREAT | O_EXCL | O_TRUNC | O_ACCMODE))
		return (-ENOTSUP);

	/* Invalid access permission flags. */
	if (mode & ~(S_IRUSR | S_IWUSR))
		return (-ENOTSUP);

	/* Missing flags? */
	if (!(oflags & O_CREAT))
		return (-EINVAL);

	/* Already opened? */
	if ((shmid = shm_get(name)) >= 0)
	{
		/* Exclusive shared memory region. */
		if (oflags & O_EXCL)
		{
			shm_put(shmid);
			return (-EEXIST);
		}

		/* Truncate. */
		if (oflags & O_TRUNC)
		{
			int ret;

			/* Cannot write. */
			if (!(oflags & (O_RDWR | O_WRONLY)))
			{
				shm_put(shmid);
				return (-EACCES);
			}

			/* Truncate. */
			if ((ret = __do_shm_ftruncate(page, proc, shmid, 0)) < 0)
			{
				shm_put(shmid);
				return (-EAGAIN);
			}
		}

		goto out;
	}

	/* Allocate shared memory region. */
	if ((shmid = shm_alloc()) < 0)
		return (-ENFILE);

	shm_set_size(shmid, 0);
	shm_set_perm(shmid, mode);
	shm_set_owner(shmid, proc);
	shm_set_name(shmid, name);


out:
	*page = regions[shmid].page;
	return (shmid);
}

/*============================================================================*
 * __do_shm_close()                                                           *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __do_shm_close(pid_t proc, int shmid)
{
	shm_debug("close proc=%d shmid=%d", proc, shmid);

	/* Invalid shared memory region. */
	if (!nanvix_shm_is_valid(shmid))
		return (-EINVAL);

	/* Bad shared memory. */
	if (!resource_is_used(&regions[shmid].resource))
		return (-EINVAL);

	/*
	 * TODO check if this process can close the shared memory region.
	 */
	((void) proc);

	return (shm_put(shmid));
}

/*============================================================================*
 * do_unlink()                                                                *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __do_shm_unlink(pid_t proc, const char *name)
{
	int shmid;

	shm_debug("unlink proc=%d name=%s", proc, name);

	/* Shared memory region does not exist. */
	if ((shmid = shm_get(name)) < 0)
		return (-EINVAL);

	/* Does the process own the shared memory region? */
	if (!shm_is_owner(shmid, proc))
	{
		shm_put(shmid);
		return (-EPERM);
	}

	shm_set_remove(shmid);

	return (__do_shm_close(proc, shmid));
}

/*============================================================================*
 * oshm_init()                                                                *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
void shm_init(void)
{
	for (int i = 0; i < NANVIX_SHM_MAX; i++)
	{
		regions[i].refcount = 0;
		regions[i].resource = RESOURCE_INITIALIZER;
		regions[i].page = RMEM_NULL;
	}
}
