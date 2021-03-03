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
#include <nanvix/types/vfs.h>
#include <posix/sys/types.h>
#include <posix/unistd.h>
#include <posix/errno.h>

/**
 * @brief Gets the block address for a file offset
 *
 * The get_blk_buf function writes in the @p buf
 * the block belonging to @p ip in which @p off is.
 *
 * @returns Upon success it returns the ordinal number of the block
 * If EOF is found 0 is returned.
 * Upon Failure an error code is returned.
 */
block_t get_blk_buf(struct inode *ip, struct buffer *blk_buf, off_t off)
{
	block_t blk;                /* offset block                   */
	block_t blk_i;              /* offset block on indirect zones */
	struct buffer *blk_b;       /* block buffer                   */
	struct buffer *blk_b_i;     /* block buffer indirect          */
	struct d_inode *ino_data;   /* inode data                     */

 	ino_data = inode_disk_get(ip);

	/* calculate in which block the offset starts */
	blk = off/NANVIX_FS_BLOCK_SIZE;

	/* access the zones in a flat way so blk indexes the right block */
	if (blk < MINIX_NR_ZONES_DIRECT) {
		/* Read bytes directly */
		if ((blk_b = bread(ip->dev, ino_data->i_zones[blk])) == NULL)
			return 0;
	}

	/* indirect */
	else if (blk >= MINIX_ZONE_SINGLE && blk < (MINIX_ZONE_SINGLE + MINIX_NR_SINGLE)) {
		/* Read bytes from single indirect blocks */
		blk_i = blk - MINIX_ZONE_SINGLE;

		if ((blk_b = bread(ip->dev, ino_data->i_zones[blk])) == NULL)
			return 0; /* EOF */

		if ((blk_b_i = bread(ip->dev, blk_i)) == NULL)
			return 0; /* EOF */

		brelse(blk_b);
		blk_b = blk_b_i;
	}

	/* Read bytes from double indirect */
	else if (blk >= (MINIX_ZONE_DOUBLE + MINIX_NR_SINGLE) && blk < MINIX_NR_DOUBLE) {
		return -ENOTSUP;
	}

	/* offset invalid */
	else {
		return MINIX_BLOCK_NULL;
	}

	umemcpy(blk_buf, blk_b, buffer_get_size());
	return blk;
}

/**
 * @brief Reads a file
 *
 * The file_read function reads @p n bytes starting from @p off
 * offset from the file @p ip to the buffer @p buf.
 * @returns Upon success the number of bytes read is returned.
 * Upon Failure an error code is returned.
 *
 * @author Lucca Augusto
 */
ssize_t do_file_read(struct inode *ip, void *buf, size_t n, off_t off)
{
	ssize_t count;              /* number of bytes read           */
	ssize_t to_read;            /* number of bytes to be read     */
	off_t local_off;            /* in block offset                */
	char *buf_data;             /* block buffer data              */
	struct buffer *blk_buf;     /* block buffer                   */

	to_read = n;
	count = 0;

	/* allocate n bytes to the buffer */
	buf = umalloc(n);
	blk_buf = umalloc(buffer_get_size());
	/* calculate in block offset */
	local_off = off%NANVIX_FS_BLOCK_SIZE;

	/* be able to read all blocks making blk++ if a really
	* large number of bytes is to be read */
	while ((size_t)count < n) {

		/* gets block in which byte offset+count is */
		if (get_blk_buf(ip, blk_buf, off+count) <= 0)
			goto endoffile;

		buf_data = buffer_get_data(blk_buf);

		/* if n spans across multiple blocks */
		if ((local_off+n) > NANVIX_FS_BLOCK_SIZE)
			to_read = (local_off+n)-NANVIX_FS_BLOCK_SIZE;

		/* copy bytes to buffer */
		for (int i=0; i<count; ++i)
			buf++;
		for (int i=0; i<local_off; ++i)
			buf_data++;

		umemcpy(buf, buf_data, to_read);

		/* count bytes read */
		count+=to_read;
		/* update number of bytes yet to be read */
		to_read = n-to_read;

		/* move to the next block */
		if ((size_t)count < n) {
			local_off = 0;
		}

	}

	return count;

endoffile:
	/* free unused memory */
	if (count == 0)
		ufree(buf);
	else
		buf = urealloc(buf, count);

	return count;
}

/*
 * See do_file_read
 */
ssize_t file_read(struct inode *ip, void *buf, size_t n, off_t off)
{
	/* Bad file descriptor */
	if (ip == NULL)
	{
		return -EBADF;
	}

	/* Directory. */
	else if (S_ISDIR(inode_disk_get(ip)->i_mode))
	{
		return -EISDIR;
	}

	/* invalid offset */
	else if (off > inode_disk_get(ip)->i_size)
	{
		return -EINVAL;
	}

	/* truncate n if off+n is bigger than file */
	if ((off+n) > inode_disk_get(ip)->i_size)
		n = (off+n) - inode_disk_get(ip)->i_size;

	return do_file_read(ip, buf, n, off);

}

/**
 * @brief Writes to a file
 *
 * The file_write function writes @p n bytes starting from @p off
 * offset from the buffer @p buf to the file @p ip.
 * @returns Upon success the number of bytes written is returned.
 * Upon Failure an error code is returned.
 *
 * @author Lucca Augusto
 */
ssize_t do_file_write(struct inode *ip, void *buf, size_t n, off_t off)
{
	ssize_t count;              /* number of bytes written        */
	ssize_t to_write;           /* number of bytes to be written  */
	off_t local_off;            /* in block offset                */
	struct buffer *blk_buf;     /* block buffer                   */
	char *buf_data;             /* buffer data                    */


	count = 0;
	to_write = n;
	local_off = off%NANVIX_FS_BLOCK_SIZE;
	blk_buf = umalloc(buffer_get_size());

	/* TODO check if there is space left */
	if (inode_disk_get(ip)->i_size + n >
			(NANVIX_FS_BLOCK_SIZE * MINIX_NR_ZONES_DIRECT +
			 NANVIX_FS_BLOCK_SIZE * MINIX_NR_SINGLE * MINIX_NR_ZONES_DIRECT +
			 NANVIX_FS_BLOCK_SIZE * MINIX_NR_SINGLE * MINIX_NR_DOUBLE * MINIX_NR_ZONES_DIRECT))
		return -ENOSPC;

	while((size_t)count < n) {

		/* block unavailable */
		if (get_blk_buf(ip, blk_buf, off+count) <= 0)
			return count;

		buf_data = buffer_get_data(blk_buf);

		/* if n spans across multiple blocks */
		if ((local_off+to_write) > NANVIX_FS_BLOCK_SIZE)
			to_write = (local_off+to_write)-NANVIX_FS_BLOCK_SIZE;

		/* copy bytes from buf to buffer data */
		for (int i=0; i<local_off; ++i)
			buf_data++;
		for (int i=0; i<count; ++i)
			buf++;
		umemcpy(buf_data, buf, to_write);

		count+=to_write;
		to_write = n-count;

		/* move to the next block */
		if ((size_t)count < n) {
			local_off = 0;
		}
	}

	/* write block buffer to device */
	bwrite(blk_buf);

	/* free block buffer */
	brelse(blk_buf);

	return count;
}

/**
 * see do_file_write
 */
ssize_t file_write(struct inode *ip, void *buf, size_t n, off_t off)
{
	/* Bad file descriptor */
	if (ip == NULL)
	{
		return -EBADF;
	}

	/* Directory. */
	else if (S_ISDIR(inode_disk_get(ip)->i_mode))
	{
		return -EISDIR;
	}

	/* invalid offset */
	else if (off > inode_disk_get(ip)->i_size)
	{
		return -EINVAL;
	}


	return do_file_write(ip, buf, n, off);
}
