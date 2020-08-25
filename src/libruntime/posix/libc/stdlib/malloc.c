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

#include <nanvix/runtime/mm.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>
#include <posix/stddef.h>

/**
 * @brief expand() in at least NALLOC blocks.
 */
#define BLOCK_SIZE 512

/**
 * @brief Size of the meta-information of a block.
 */
#define BLOCK_STRUCT_SIZE (sizeof(struct block))

/**
 * @brief Real size of the block (metadata + user data).
 */
#define BLOCK_META_SIZE(_size) (BLOCK_STRUCT_SIZE + _size)

/**
 * @brief Minimum size of block in bytes (metadata + 1 char).
 */
#define BLOCK_MIN_SIZE (BLOCK_META_SIZE(sizeof(char)))

/**
 * @brief Memory block.
 */
struct block
{
	struct block *nextp; /* Next free block.  */
	size_t size;         /* Size (in bytes). */
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
 *
 * @bug FIXME In some targets, like MPPA-256, the TLB shootdown
 * is not supported. So, for now, it is unsafe to effectively release
 * underlying memory. See issue:
 * https://github.com/nanvix/multikernel/issues/214.
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
	for (p = freep; !(p <= bp && bp <= p->nextp); p = p->nextp)
	{
		/* Freed block at start or end. */
		if (p >= p->nextp && (p < bp || bp < p->nextp))
			break;
	}

	/* Merge with upper block. */
	if (bp + bp->size == p->nextp)
	{
		bp->size += p->nextp->size;
		bp->nextp = p->nextp->nextp;
	}
	else
		bp->nextp = p->nextp;

	/* Merge with lower block. */
	if (p + p->size == bp)
	{
		p->size += bp->size;
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
static void *expand(size_t size)
{
	size_t n;
	struct block *p;

	n = TRUNCATE(size, PAGE_SIZE)/PAGE_SIZE;

	/* Request more memory to the kernel. */
	if ((p = nanvix_vmem_alloc(n)) == NULL)
		return (NULL);

	p->size = n*PAGE_SIZE;
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
	size_t bsize;        /* Requested block size.     */
	struct block *p;     /* Working block.            */
	struct block *q;     /* Auxiliar block.           */
	struct block *prevp; /* Previous working block.   */

	/* Nothing to be done. */
	if (size == 0)
		return (NULL);

	/* Create free list. */
	if ((prevp = freep) == NULL)
	{
		head.nextp = freep = prevp = &head;
		head.size = 0;
	}

	bsize = BLOCK_META_SIZE(size);

	/* Look for a free block that is big enough. */
	for (p = prevp->nextp; /* void */ ; prevp = p, p = p->nextp)
	{
		/* Found. */
		if (p->size >= bsize)
		{
			/* Exact or there isn't enough space for another block. */
			if (WITHIN(p->size, bsize, bsize + BLOCK_MIN_SIZE))
				prevp->nextp = p->nextp;

			/* Split block. */
			else
			{
				/* Gets the next block pointer. */
				q = (struct block *) (((char *) p) + bsize);

				/* Sets remaining size. */
				q->size = (p->size - bsize);

				/* Puts new block into free list. */
				prevp->nextp = q;
				q->nextp     = p->nextp;

				/* Updates size of allocated block. */
				p->size = bsize;
			}

			freep = prevp;

			return (p + 1);
		}

		/* Wrapped around free list. */
		if (p == freep)
		{
			/* Expand heap. */
			if ((p = expand(bsize)) == NULL)
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
		return (NULL);

	newptr = nanvix_malloc(size);

	/* Checks if there are more operations to be done. */
	if (ptr != NULL)
	{
		umemcpy(newptr, ptr, size);

		nanvix_free(ptr);
	}

	return (newptr);
}
