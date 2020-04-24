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

#define __NEED_RMEM_CACHE

#include <nanvix/runtime/rmem.h>
#include <nanvix/sys/page.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/stddef.h>
#include <posix/stdint.h>

/**
 * @brief Length of remote memory table.
 */
#define RMEM_TABLE_LENGTH 1024

/**
 * @brief Computes a remote address.
 */
#define RADDR(x) (UBASE_VIRT + ((x) << RMEM_BLOCK_SHIFT))

/**
 * @brief Computes a remote address (reverse).
 */
#define RADDR_INV(x) ((vaddr_t)(x) - UBASE_VIRT)

/**
 * @brief Remote memory table
 */
static rpage_t rmem_table[RMEM_TABLE_LENGTH] = {
	[0 ... (RMEM_TABLE_LENGTH - 1)] = RMEM_NULL
};

/**
 * @brief Current break value for remote memory.
 */
static int rbrk = 1;

/*============================================================================*
 * nanvix_vmem_lookup()                                                       *
 *============================================================================*/

/**
 * @brief Looks up a remote memory address.
 *
 * @param base   Store location for base address.
 * @param offset Store location for offset address.
 * @param ptr    Remote memory address.
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, a negative error code is returned instead.
 */
static int nanvix_vmem_lookup(raddr_t *base, raddr_t *offset, const void *ptr)
{
	raddr_t _base;
	raddr_t _offset;

	/* Invalid remote address. */
	if (ptr == NULL)
		return (-EFAULT);

	_base = ((raddr_t) ptr) >> RMEM_BLOCK_SHIFT;

	/* Invalid remote memory area. */
	if (_base >= RMEM_TABLE_LENGTH)
		return (-EINVAL);

	/* Bad remote memory address. */
	if (rmem_table[_base] == RMEM_NULL)
		return (-EFAULT);

	_offset = ((raddr_t) ptr) & (RMEM_BLOCK_SIZE - 1);

	if (base != NULL)
		*base = _base;
	if (offset != NULL)
		*offset = _offset;

	return (0);
}

/*============================================================================*
 * nanvix_vmem_expand()                                                       *
 *============================================================================*/

/**
 * @brief Increases the break value of the remote memory.
 *
 * @param n Number of block to expand.
 *
 * @returns Upon successful completion, the previous break value of the
 * remote memory is returned. Upon failure, a negative error code is
 * returned instead.
 */
static int nanvix_vmem_expand(int n)
{
	int old_rbrk;

	/* Invalid heap increase. */
	if (n <= 0)
		return (-EINVAL);

	/* Not enough memory.*/
	if ((rbrk + n) >= RMEM_TABLE_LENGTH)
		return (-ENOMEM);

	/* Increase heap value. */
	old_rbrk = rbrk;
	rbrk += n;

	return (old_rbrk);
}

/*============================================================================*
 * nanvix_vmem_contract()                                                     *
 *============================================================================*/

/**
 * @brief Decreases the break value of the remote memory.
 *
 * @param n Number of block to contract.
 *
 * @returns Upon successful completion, the previous break value of the
 * remote memory is returned. Upon failure, a negative error code is
 * returned instead.
 */
static int nanvix_vmem_contract(int n)
{
	int old_rbrk;

	/* Invalid heap decrease. */
	if (n <= 0)
		return (-EINVAL);

	/* Bad heap decrease. */
	if ((rbrk - n) < 1)
		return (-EINVAL);

	/* Increase heap value. */
	old_rbrk = rbrk;
	rbrk -= n;

	return (old_rbrk);
}

/*============================================================================*
 * nanvix_vmem_alloc()                                                        *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 * @todo TODO: Enable arbitrary remote memory allocation.
 */
void *nanvix_vmem_alloc(size_t n)
{
	int base;
	rpage_t pgnum;

	/* Invalid allocation size */
	if (n == 0)
		return (NULL);

	/*
	 * Find an empty slot in the
	 * remote memory table.
	 */
	if ((base = nanvix_vmem_expand(n)) < 0)
		return (NULL);

	for (size_t i = 0; i < n; i++)
	{
		/* Allocate page. */
		if ((pgnum = nanvix_rcache_alloc()) == RMEM_NULL)
			return (NULL);

		rmem_table[base + i] = pgnum;
	}


	return ((void *) RADDR(base));
}

/*============================================================================*
 * nanvix_vmem_free()                                                         *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 */
int nanvix_vmem_free(void *ptr)
{
	int err;      /* Error code.   */
	raddr_t base; /* Base address. */

	ptr = (void *)RADDR_INV(ptr);

	/* Invalid remote address. */
	if (ptr == NULL)
		return (-EFAULT);

	/* Lookup remote address. */
	if ((err = nanvix_vmem_lookup(&base, NULL, ptr)) < 0)
		return (err);

	/* Invalid address. */
	if ((rbrk - base) < 1)
		return (-EFAULT);

	for (int i = base; i < rbrk; i++)
	{
		/* Free underlying remote page. */
		if ((err = nanvix_rcache_free(rmem_table[i])) < 0)
			return (err);

		/* Update remote memory table. */
		rmem_table[i] = RMEM_NULL;
	}

	err = nanvix_vmem_contract(rbrk - base);

	return ((err < 0) ? err : 0);
}

/*============================================================================*
 * nanvix_vmem_read()                                                         *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 * @todo TODO: Enable arbitrary read offset.
 * @todo TODO: Enable arbitrary read sizes.
 */
size_t nanvix_vmem_read(void *buf, const void *ptr, size_t n)
{
	char *rptr;     /* Cached remote page. */
	int err;        /* Error code.         */
	raddr_t base;   /* Base address.       */
	raddr_t offset; /* Offset address.     */

	ptr = (void *)RADDR_INV(ptr);

	/* Nothing to do. */
	if (n == 0)
		return (0);

	/* Invalid remote address. */
	if (ptr == NULL)
	{
		errno = EFAULT;
		return (0);
	}

	/* Lookup remote address. */
	if ((err = nanvix_vmem_lookup(&base, &offset, ptr)) < 0)
	{
		errno = -err;
		return (0);
	}

	/* Invalid buffer. */
	if (buf == NULL)
	{
		errno = EINVAL;
		return (0);
	}

	/* Invalid read size. */
	if ((n > RMEM_BLOCK_SIZE) || ((offset + n) > RMEM_BLOCK_SIZE))
	{
		errno = EINVAL;
		return (0);
	}

	/* Get cached remote page. */
	if ((rptr = nanvix_rcache_get(rmem_table[base])) == NULL)
		return (0);

	umemcpy(buf, &rptr[offset], n);

	return (n);
}

/*============================================================================*
 * nanvix_vmem_write()                                                        *
 *============================================================================*/

/**
 * @todo TODO: Provide a detailed description for this function.
 * @todo TODO: Enable arbitrary write offset.
 * @todo TODO: Enable arbitrary write sizes.
 */
size_t nanvix_vmem_write(void *ptr, const void *buf, size_t n)
{
	char *rptr;     /* Cached remote page. */
	int err;        /* Error code.         */
	raddr_t base;   /* Base address.       */
	raddr_t offset; /* Offset address.     */

	ptr = (void *)RADDR_INV(ptr);

	/* Nothing to do. */
	if (n == 0)
		return (0);

	/* Invalid remote address. */
	if (ptr == NULL)
	{
		errno = EFAULT;
		return (0);
	}

	/* Lookup remote address. */
	if ((err = nanvix_vmem_lookup(&base, &offset, ptr)) < 0)
	{
		errno = -err;
		return (0);
	}

	/* Invalid buffer. */
	if (buf == NULL)
	{
		errno = EINVAL;
		return (0);
	}

	/* Invalid write size. */
	if ((n > RMEM_BLOCK_SIZE) || ((offset + n) > RMEM_BLOCK_SIZE))
	{
		errno = EINVAL;
		return (0);
	}

	/* Get cached remote page. */
	if ((rptr = nanvix_rcache_get(rmem_table[base])) == NULL)
		return (0);

	umemcpy(&rptr[offset], buf, n);

	return (n);
}

/*============================================================================*
 * nanvix_rfault()                                                            *
 *============================================================================*/

/**
 * @brief Page maps.
 */
static struct
{
	vaddr_t laddr; /**< Local address.                         */
	void *raddr;   /**< Pointer to locally-mapped remote page. */
} maps[RMEM_CACHE_SIZE] = {
	[0 ... (RMEM_CACHE_SIZE - 1)] = { RMEM_NULL, NULL }
};

/**
 * @brief Handles a page fault.
 */
int nanvix_rfault(vaddr_t vaddr)
{
	int idx = 0;  /* Idex to table of page maps.  */
	void *lptr;   /* Local pointer.               */
	void *rptr;   /* Remote pointer.              */
	raddr_t base; /* Base address of remote page. */

	vaddr &= PAGE_MASK;
	lptr = (void *)RADDR_INV(vaddr);

	/* Lookup remote address. */
	if (nanvix_vmem_lookup(&base, NULL, lptr) < 0)
		return (-EFAULT);

	/* Get cached remote page. */
	if ((rptr = nanvix_rcache_get(rmem_table[base])) == NULL)
		return (-EFAULT);

	/* Unlink old page page from there. */
	for (int i = 0; i < RMEM_CACHE_SIZE; i++)
	{
		/* Found. */
		if (maps[i].raddr == rptr)
		{
			uassert(page_unmap(maps[idx = i].laddr) == 0);
			goto done;
		}

		/* Remember this index. */
		idx = i;
	}

done:

	/* Link page. */
	maps[idx].raddr = rptr;
	maps[idx].laddr = vaddr;
	uassert(page_link((vaddr_t) rptr, (vaddr_t) vaddr) == 0);

	return (0);
}
