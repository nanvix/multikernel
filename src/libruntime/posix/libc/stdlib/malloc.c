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
#define __NEED_MM_MANAGER

#include <nanvix/runtime/rmem.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>
#include <posix/stddef.h>

/**
 * @brief expand() in at least NALLOC blocks.
 */
#define NALLOC 511

/**
 * @brief Size of block structure.
 */
#define SIZEOF_BLOCK (sizeof(struct block))

/**
 * @brief Memory block.
 */
struct block
{
	struct block *nextp; /* Next free block.  */
	unsigned nblocks;    /* Size (in blocks). */
};

/**
 * @brief Free list of blocks.
 */
static struct block head;
static struct block *freep = NULL;

/**
 * @brief Frees allocated memory.
 *
 * @param ptr Memory area to free.
 */
void nanvix_free(void *ptr)
{
	struct block *p;  /* Working block.     */
	struct block *bp; /* Block being freed. */

	/* Nothing to be done. */
	if (ptr == NULL)
		return;

	bp = (struct block *)ptr - 1;

	/* Look for insertion point. */
	for (p = freep; !(p < bp && p->nextp > bp); p = p->nextp)
	{
		/* Freed block at start or end. */
		if (p >= p->nextp && (bp > p || bp < p->nextp))
			break;
	}

	/* Merge with upper block. */
	if (bp + bp->nblocks == p->nextp)
	{
		bp->nblocks += p->nextp->nblocks;
		bp->nextp = p->nextp->nextp;
	}
	else
		bp->nextp = p->nextp;

	/* Merge with lower block. */
	if (p + p->nblocks == bp)
	{
		p->nblocks += bp->nblocks;
		p->nextp = bp->nextp;
	}
	else
		p->nextp = bp;


	freep = p;
}

/**
 * @brief Expands the heap.
 *
 * @details Expands the heap by @p nblocks.
 *
 * @param nblocks Number of blocks to expand.
 *
 * @returns Upon successful completion a pointed to the expansion is returned.
 *          Upon failure, a null pointed is returned instead and errno is set
 *          to indicate the error.
 */
static void *expand(unsigned nblocks)
{
	struct block *p;
	size_t n;

	/* Expand in at least NALLOC blocks. */
	if (nblocks < NALLOC)
		nblocks = NALLOC;

	n = TRUNCATE(nblocks*SIZEOF_BLOCK, PAGE_SIZE)/PAGE_SIZE;

	/* Request more memory to the kernel. */
	if ((p = nanvix_vmem_alloc(n)) == NULL)
		return (NULL);

	p->nblocks = nblocks;
	nanvix_free(p + 1);

	return (freep);
}

/**
 * @brief Allocates memory.
 *
 * @param size Number of bytes to allocate.
 *
 * @returns Upon successful completion with size not equal to 0, nanvix_malloc()
 *          returns a pointer to the allocated space. If size is 0, either a
 *          null pointer or a unique pointer that can be successfully passed to
 *          nanvix_free() is returned. Otherwise, it returns a null pointer and set
 *          errno to indicate the error.
 */
void *nanvix_malloc(size_t size)
{
	struct block *p;     /* Working block.            */
	struct block *prevp; /* Previous working block.   */
	unsigned nblocks;    /* Request size (in blocks). */

	/* Nothing to be done. */
	if (size == 0)
		return (NULL);

	nblocks = (size + (SIZEOF_BLOCK - 1))/SIZEOF_BLOCK + 1;

	/* Create free list. */
	if ((prevp = freep) == NULL)
	{
		head.nextp = freep = prevp = &head;
		head.nblocks = 0;
	}

	/* Look for a free block that is big enough. */
	for (p = prevp->nextp; /* void */ ; prevp = p, p = p->nextp)
	{
		/* Found. */
		if (p->nblocks >= nblocks)
		{
			/* Exact. */
			if (p->nblocks == nblocks)
				prevp->nextp = p->nextp;

			/* Split block. */
			else
			{
				p->nblocks -= nblocks;
				p += p->nblocks;
				p->nblocks = nblocks;
			}

			freep = prevp;

			return (p + 1);
		}

		/* Wrapped around free list. */
		if (p == freep)
		{
			/* Expand heap. */
			if ((p = expand(nblocks)) == NULL)
				break;
		}
	}

	return (NULL);
}

/**
 * @brief Reallocates a memory chunk.
 *
 * @param ptr  Pointer to old object.
 * @param size Size of new object.
 *
 * @returns Upon successful completion, nanvix_realloc() returns a pointer to the
 *           allocated space. Upon failure, a null pointer is returned instead.
 *
 * @todo Check if we can simply expand.
 */
void *nanvix_realloc(void *ptr, size_t size)
{
	void *newptr;

	/* Nothing to be done. */
	if (size == 0)
	{
		errno = EINVAL;
		return (NULL);
	}

	newptr = nanvix_malloc(size);
	if (ptr != NULL)
		umemcpy(newptr, ptr, size);

	nanvix_free(ptr);

	return (newptr);
}
