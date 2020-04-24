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

#include <nanvix/runtime/utils.h>

/**
 * @todo TODO: provide a detailed description for this function.
 */
bitmap_t bitmap_nset(bitmap_t *bitmap, size_t size)
{
	bitmap_t count; /* Number of bits set. */
	bitmap_t *idx;  /* Loop index.         */
	bitmap_t *end;  /* End of bitmap.      */
	bitmap_t chunk; /* Working chunk.      */

	/**
	 * @brief Assert bitmap size.
	 */
	((void) sizeof(char[(((sizeof(bitmap_t)) == (BITMAP_WORD_LENGTH/8)) ? 1 : -1)]));

	/* Count the number of bits set. */
	count = 0;
	end = (bitmap + (size >> 2));
	for (idx = bitmap; idx < end; idx++)
	{
		chunk = *idx;

		/*
		 * Fast way for counting number of bits set in a bit map.
		 * I have no idea how does it work. I just got it from here:
		 * https://graphics.stanford.edu/~seander/bithacks.html
		 */
		chunk = chunk - ((chunk >> 1) & 0x55555555);
		chunk = (chunk & 0x33333333) + ((chunk >> 2) & 0x33333333);
		count += (((chunk + (chunk >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
	}

	return (count);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
bitmap_t bitmap_nclear(bitmap_t *bitmap, size_t size)
{
	return ((size << 3) - bitmap_nset(bitmap, size));
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
bitmap_t bitmap_first_free(bitmap_t *bitmap, size_t size)
{
    bitmap_t *max;          /* Bitmap bondary. */
    register bitmap_t off;  /* Bit offset.     */
    register bitmap_t *idx; /* Bit index.      */

    idx = bitmap;
    max = (idx + (size >> 2));

    /* Find bit index. */
    while (idx < max)
    {
		/* Index found. */
		if (*idx != 0xffffffff)
		{
			off = 0;

			/* Find offset. */
			while (*idx & (0x1 << off))
				off++;

			return (((idx - bitmap) << BITMAP_WORD_SHIFT) + off);
		}

		idx++;
	}

	return (BITMAP_FULL);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
bitmap_t bitmap_check_bit(bitmap_t *bitmap, bitmap_t idx)
{
	return (bitmap[IDX(idx)] & (1 << OFF(idx)));
}
