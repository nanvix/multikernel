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
#include <nanvix/dev.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

/**
 * @brirf Number of Block address per Block
 */
#define MINIX_BLOCK_ADDRS_PER_BLOCK (MINIX_BLOCK_SIZE/sizeof(minix_block_t))

/*============================================================================*
 * minix_block_alloc()                                                        *
 *============================================================================*/

/**
 * The minix_block_alloc() function allocates a file system block in the MINIX
 * file system pointed to by @p sb. Disk block is allocated from the
 * zone map pointed to by @p zmap.
 */
minix_block_t minix_block_alloc(
	const struct d_superblock *sb,
	bitmap_t *zmap
)
{
	bitmap_t bit;

	/* Invalid superblock. */
	if (sb == NULL)
		return (MINIX_BLOCK_NULL);

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (MINIX_BLOCK_NULL);

	/* Invalid zone map. */
	if (zmap == NULL)
		return (MINIX_BLOCK_NULL);

	/* Allocate block. */
	bit = bitmap_first_free(zmap, sb->s_bmap_nblocks*MINIX_BLOCK_SIZE);
	if (bit == BITMAP_FULL)
	{
		uprintf("[nanvix][vfs] minix block overflow");
		return (MINIX_BLOCK_NULL);
	}

	bitmap_set(zmap, bit);

	return (sb->s_first_data_block + bit);
}

/*============================================================================*
 * minix_block_map()                                                          *
 *============================================================================*/

/**
 * @brief The minix_block_map_alloc() function maps the byte offset @p
 * off in the file pointed to by @p ip. The file system block is mapped in the
 * MINIX file system pointed to by @p sb and allocated in the zone map
 * pointed to by @p zmap if required.
 */
minix_block_t minix_block_map(
	struct d_superblock *sb,
	bitmap_t *zmap,
	struct d_inode *ip,
	off_t off,
	int create
)
{
	minix_block_t phys;                             /* Phys. blk. #.   */
	minix_block_t logic;                            /* Logic. blk. #.  */
	minix_block_t buf[MINIX_BLOCK_ADDRS_PER_BLOCK]; /* Working buffer. */

	logic = off/MINIX_BLOCK_SIZE;

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (MINIX_BLOCK_NULL);

	/* File offset too big. */
	if ((bitmap_t)off >= sb->s_max_size)
		return (MINIX_BLOCK_NULL);

	/*
	 * Create blocks that are
	 * in a valid offset.
	 */
	if ((bitmap_t)off < ip->i_size)
		create = true;

	/* Direct block. */
	if (logic < MINIX_NR_ZONES_DIRECT)
	{
		/* Create direct block. */
		if (ip->i_zones[logic] == MINIX_BLOCK_NULL && create)
		{
			phys = minix_block_alloc(sb, zmap);
			ip->i_zones[logic] = phys;
		}

		return (ip->i_zones[logic]);
	}

	logic -= MINIX_NR_ZONES_DIRECT;

	/* Single indirect block. */
	if (logic < MINIX_NR_SINGLE)
	{
		/* Create single indirect block. */
		if (ip->i_zones[MINIX_ZONE_SINGLE] == MINIX_BLOCK_NULL && create)
		{
			phys = minix_block_alloc(sb, zmap);
			ip->i_zones[MINIX_ZONE_SINGLE] = phys;
		}

		/* We cannot go any further. */
		if ((phys = ip->i_zones[MINIX_ZONE_SINGLE]) == MINIX_BLOCK_NULL)
			upanic("invalid offset");

		off = phys*MINIX_BLOCK_SIZE;
		bdev_read(0, (char *)buf, MINIX_BLOCK_SIZE, off);

		/* Create direct block. */
		if (buf[logic] == MINIX_BLOCK_NULL && create)
		{
			phys = minix_block_alloc(sb, zmap);
			buf[logic] = phys;
			bdev_write(0, (char *) buf, MINIX_BLOCK_SIZE, off);
		}

		return (buf[logic]);
	}

	logic -= MINIX_NR_SINGLE;

	/* Double indirect zone. */
	upanic("double indect zone");

	return (MINIX_BLOCK_NULL);
}

/*============================================================================*
 * minix_minix_block_free()                                                   *
 *============================================================================*/

/**
 * The minix_block_free_direct() function releases the direct file
 * system block @p num that resides in the MINIX file system pointed to
 * by @p sb and is allocated in the zone map pointed by @p zmap.
 */
int minix_block_free_direct(
	struct d_superblock *sb,
	bitmap_t *zmap,
	minix_block_t num
)
{
	bitmap_t bit;

	/* Inalid superblock. */
	if (sb == NULL)
		return (-EINVAL);

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (-EINVAL);

	/* Invalid zone map. */
	if (zmap == NULL)
		return (-EINVAL);

	/* Nothing to do. */
	if (num == MINIX_BLOCK_NULL)
		return (-EINVAL);

	num -= sb->s_first_data_block;

	/* Invalid block. */
	if (num >= sb->s_nblocks)
		return (-EINVAL);

	bit = num;

	/* Free  file system block. */
	bitmap_clear(zmap, bit);

	return (0);
}

/**
 * The minix_block_free_indirect() function releases the indirect block
 * disk @p num that resides in the MINIX file system pointed to by @p sb
 * and is allocated in the zone map pointed by @p zmap.
 */
int minix_block_free_indirect(
	struct d_superblock *sb,
	bitmap_t *zmap,
	minix_block_t num
)
{
	/* Inalid superblock. */
	if (sb == NULL)
		return (-EINVAL);

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (-EINVAL);

	/* Invalid zone map. */
	if (zmap == NULL)
		return (-EINVAL);

	/* Nothing to be done. */
	if (num == MINIX_BLOCK_NULL)
		return (-EINVAL);

	/* Free indirect  file system block. */
	for (unsigned i = 0; i < MINIX_NR_SINGLE; i++)
		minix_block_free_direct(sb, zmap, num);

	minix_block_free_direct(sb, zmap, num);

	return (0);
}

/**
 * The minix_block_free_dindirect() function releases the double
 * indirect file system block @p num that resides in the MINIX file
 * system pointed to by @p sb and is allocated in the zone map pointed
 * by @p zmap.
 */
int minix_block_free_dindirect(
	struct d_superblock *sb,
	bitmap_t *zmap,
	minix_block_t num
)
{
	/* Inalid superblock. */
	if (sb == NULL)
		return (-EINVAL);

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (-EINVAL);

	/* Invalid zone map. */
	if (zmap == NULL)
		return (-EINVAL);

	/* Nothing to be done. */
	if (num == MINIX_BLOCK_NULL)
		return (-EINVAL);

	/* Free direct zone. */
	for (unsigned i = 0; i < MINIX_NR_SINGLE; i++)
		minix_block_free_indirect(sb, zmap, num);

	minix_block_free_direct(sb, zmap, num);

	return (0);
}

/**
 * The minix_block_free() function releases the double indirect file
 * system block @p num that resides in the MINIX file system pointed to
 * by @p sb and is allocated in the zone map pointed by @p zmap.
 */
int minix_block_free(
	struct d_superblock *sb,
	bitmap_t *zmap,
	minix_block_t num,
	int lvl
)
{
	/* Inalid superblock. */
	if (sb == NULL)
		return (-EINVAL);

	/* Bad superblock. */
	if (sb->s_magic != MINIX_SUPER_MAGIC)
		return (-EINVAL);

	/* Invalid zone map. */
	if (zmap == NULL)
		return (-EINVAL);

	/* Release file system block. */
	switch (lvl)
	{
		/* Direct block. */
		case 0:
			minix_block_free_direct(sb, zmap, num);
			break;

		/* Single indirect block. */
		case 1:
			minix_block_free_indirect(sb, zmap, num);
			break;

		/* Doubly indirect block. */
		case 2:
			minix_block_free_dindirect(sb, zmap, num);
			break;

		/* Should not happen. */
		default:
			return (-EINVAL);
	}

	return (0);
}
