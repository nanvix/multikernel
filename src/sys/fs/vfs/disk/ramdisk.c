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

#include <dev/ramdisk.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/errno.h>

/**
 * @brief RAM Disk
 */
static struct
{
	char data[NANVIX_RAMDISK_SIZE]; /**< Underlying Data */
} ramdisks[NANVIX_NR_RAMDISKS];

/**
 * The ramdisk_write() function writes @p n bytes from the buffer
 * pointed to by @p buf in the ramdisk device @p minor at offset @p off.
 */
ssize_t ramdisk_write(unsigned minor, const char *buf, size_t n, off_t off)
{
	char *ptr; /* Write pointer.    */

	/* Invalid device. */
	if (minor >= NANVIX_NR_RAMDISKS)
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid offset. */
	if (off < 0)
		return (-EINVAL);

	/* Invalid write size. */
	if (n > NANVIX_RAMDISK_SIZE)
		return (-EINVAL);

	ptr = ramdisks[minor].data + off;

	/* Invalid offset. */
	if (ptr >= (ramdisks[minor].data + NANVIX_RAMDISK_SIZE))
		return (-EINVAL);

	/* Invalid write size. */
	if ((ptr + n) > (ramdisks[minor].data + NANVIX_RAMDISK_SIZE))
		return (-EINVAL);

	umemcpy(ptr, buf, n);

	return ((ssize_t) n);
}

/**
 * The ramdisk_read() function reads @p n bytes from the ramdisk device
 * @p minor at offset @p off to the buffer pointed to by @p buf.
 */
ssize_t ramdisk_read(unsigned minor, char *buf, size_t n, off_t off)
{
	char *ptr; /* Write pointer.   */

	/* Invalid device. */
	if (minor >= NANVIX_NR_RAMDISKS)
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid offset. */
	if (off < 0)
		return (-EINVAL);

	/* Invalid read size. */
	if (n > NANVIX_RAMDISK_SIZE)
		return (-EINVAL);

	ptr = ramdisks[minor].data + off;

	/* Invalid offset. */
	if (ptr >= (ramdisks[minor].data + NANVIX_RAMDISK_SIZE))
		return (-EINVAL);

	/* Invalid read size. */
	if ((ptr + n) > (ramdisks[minor].data + NANVIX_RAMDISK_SIZE))
		return (-EINVAL);

	umemcpy(buf, ptr, n);

	return ((ssize_t) n);
}

/**
 * The ramdisk_init() function initializes ramdisk devices.
 */
void ramdisk_init(void)
{
	uprintf("[nanvix][dev] initializing ramdisk device driver");

	for (unsigned i = 0; i < NANVIX_NR_RAMDISKS; i++)
		umemset(ramdisks[i].data, 0, NANVIX_RAMDISK_SIZE);
}
