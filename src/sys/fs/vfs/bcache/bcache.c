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
#define __NEED_RESOURCE

#include <nanvix/hal/resource.h>
#include <nanvix/servers/vfs.h>
#include <nanvix/config.h>
#include <nanvix/dev.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>

/**
 * @brief Block Buffer
 */
struct buffer
{
	/**
	 * @name Status information
	 *
	 * @note Must come first.
	 */
	/**@{*/
	struct resource flags; /**< Flags */
	/**@}*/

	/**
	 * @name General information
	 */
	/**@{*/
	dev_t dev;                        /**< Device.          */
	block_t num;                      /**< Block number.    */
	char data[NANVIX_FS_BLOCK_SIZE];  /**< Underlying data. */
	int count;                        /**< Reference count. */
	/**@}*/
};

/**
 * @brief Block buffers.
 */
static struct buffer buffers[NANVIX_FS_NR_BUFFERS];

/**
 * @brief Returns the size in bytes of the buffer struct
 */
int buffer_get_size(void)
{
	return sizeof(struct buffer);
}

/**
 * The buffer_get_data() function gets a reference to the underlying
 * data of the block buffer pointed to by @p buf.
 */
void *buffer_get_data(struct buffer *buf)
{
	/* Invalid buffer. */
	if (buf == NULL)
		return (NULL);

	/* Bad buffer. */
	if ((buf < &buffers[0]) || (buf >= &buffers[NANVIX_FS_NR_BUFFERS]))
		return (NULL);


	return (buf->data);
}

/**
 * The buffer_set_dirty() sets the block buffer pointed to by @p buf as
 * dirty.
 */
int buffer_set_dirty(struct buffer *buf)
{
	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Bad buffer. */
	if ((buf < &buffers[0]) || (buf >= &buffers[NANVIX_FS_NR_BUFFERS]))
		return (-EINVAL);
	resource_set_dirty(&buf->flags);

	return (0);
}

/**
 * The buffer_is_dirty() asserts whether or not the buffer pointed to by
 * @p buf is dirty.
 */
int buffer_is_dirty(struct buffer *buf)
{
	/* Invalid buffer. */
	if (buf == NULL)
		return (0);

	/* Bad buffer. */
	if ((buf < &buffers[0]) || (buf >= &buffers[NANVIX_FS_NR_BUFFERS]))
		return (0);

	return (resource_is_dirty(&buf->flags));
}

/**
 * @brief Evits a block from the block cache.
 *
 * @returns Upon successful completion, a pointer to a free block buffer
 * is returned. Else, a NULL pointer is returned instead.
 */
static struct buffer *evict(void)
{
	static int k = 0;          /* Starting Searching Position */
	struct buffer *buf = NULL; /* Selected Buffer             */

	/* Evict. */
	for (int i = k; i !=  (k - 1)%NANVIX_FS_NR_BUFFERS; i = (i + 1)%NANVIX_FS_NR_BUFFERS)
	{
		/* Skip used. */
		if (resource_is_used(&buffers[i].flags))
			continue;

		/* Dirty block is a candidate. */
		if (resource_is_dirty(&buffers[i].flags))
		{
			buf = &buffers[i];
			continue;
		}

		buf = &buffers[i];
		break;
	}

	k = (k + 1)%NANVIX_FS_NR_BUFFERS;

	/* No buffer is available. */
	if (buf == NULL)
		return (NULL);

	/*
	 * Write-back buffer ? Note that disk operations
	 * are not blocking for now, so we are safe.
	 */
	if (resource_is_dirty(&buf->flags))
	{
		bdev_writeblk(buf);
		resource_set_clean(&buf->flags);
	}

	resource_set_invalid(&buf->flags);

	return (buf);
}

/**
 * @brief Gets a block buffer from the block cache.
 *
 * @param dev Number of target device.
 * @param num Number of target lcok.
 *
 * @returns Upon successful completion, a pointer to a buffer holding
 * the requested block is returned. In this case, the block buffer is
 * ensured to be locked, and may be, or may be not, valid.  Upon
 * failure, a null pointer NULL is returned instead.
 */
static struct buffer *getblk(dev_t dev, block_t num)
{
	struct buffer *buf = NULL;

	/* Search target block. */
	for (int i = 0; i < NANVIX_FS_NR_BUFFERS; i++)
	{
		/* Skip dirty. */
		if (!resource_is_valid(&buffers[i].flags))
			continue;

		/* Found. */
		if ((buffers[i].dev == dev) && (buffers[i].num == num))
		{
			buf = &buffers[i];
			goto found;
		}
	}

	/* Evict. */
	if ((buf = evict()) == NULL)
		return (NULL);

	buf->dev = dev;
	buf->num = num;

found:

	buf->count++;
	resource_set_used(&buf->flags);

	return (buf);
}

/**
 * The brelse function releases the block poitned to by @p buf. If its
 * reference count drops to zero, the block buffer is put into the block
 * cache.
 */
int brelse(struct buffer *buf)
{
	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Bad buffer. */
	if ((buf < &buffers[0]) || (buf >= &buffers[NANVIX_FS_NR_BUFFERS]))
		return (-EINVAL);

	/* Bad buffer. */
	if (buf->count == 0)
		return (-EINVAL);

	/* Release buffer. */
	if (buf->count-- == 1)
		resource_set_unused(&buf->flags);

	return (0);
}

/**
 * The bread() function reads the block @p num from the device @p dev
 * into the block cached. Upon successful completion, a pointer to a
 * buffer that holds the requested block is returned.
 */
struct buffer *bread(dev_t dev, block_t num)
{
	struct buffer *buf = NULL;

	/* Get block buffer. */
	if ((buf = getblk(dev, num)) == NULL)
		return (NULL);

	/* Read-in block. */
	if (!resource_is_valid(&buf->flags))
	{
		if (bdev_readblk(buf) < 0)
		{
			uassert(brelse(buf) == 0);
			return (NULL);
		}

		resource_set_valid(&buf->flags);
	}

	return (buf);
}

/**
 * The bwrite2() function writes the block buffer pointed to by buf to
 * the underlying device.
 */
int bwrite2(struct buffer *buf)
{
	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Bad buffer. */
	if ((buf < &buffers[0]) || (buf >= &buffers[NANVIX_FS_NR_BUFFERS]))
		return (-EINVAL);

	/* Bad buffer. */
	if (buf->count == 0)
		return (-EINVAL);

	/* Write back. */
	if (resource_is_valid(&buf->flags))
	{
		if (resource_is_dirty(&buf->flags))
		{
			bdev_writeblk(buf);
			resource_set_clean(&buf->flags);
		}
	}

	return (0);
}

/**
 * The bwrite() function writes the block buffer pointed to by buf to
 * the underlying device. Once the operation is completed, the buffer is
 * released.
 */
int bwrite(struct buffer *buf)
{
	int err;

	/* Write buffer. */
	if ((err = bwrite2(buf)) < 0)
		return (err);

	return (brelse(buf));
}

/**
 * The binit() function initializes the block cache. It places all block
 * buffers in the free list and cleans the hash table of block buffers.
 */
void binit(void)
{
	uprintf("[nanvix][vfs] initializing block cache...");

	/* Initialize buffers. */
	for (int i = 0; i < NANVIX_FS_NR_BUFFERS; i++)
	{
		buffers[i].flags = RESOURCE_INITIALIZER;
		buffers[i].dev = 0;
		buffers[i].num = 0;
		buffers[i].count = 0;
	}

	uprintf("[nanvix][vfs] %d slots in the block cache", NANVIX_FS_NR_BUFFERS);
}
