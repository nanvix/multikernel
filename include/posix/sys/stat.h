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

#ifndef POSIX_SYS_STAT_H_
#define POSIX_SYS_STAT_H_

#ifndef __unix64__

	#include <posix/sys/types.h>

	/* File types. */
    #ifndef __APPLE__
	    #define S_IFMT  00170000
    #endif /* __APPLE__ */
	#define S_IFREG  0100000
	#define S_IFBLK  0060000
	#define S_IFDIR  0040000
	#define S_IFCHR  0020000
	#define S_IFIFO  0010000

	#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG) /* Regular file?       */
	#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR) /* Directory?          */
	#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR) /* Char. special file? */
	#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK) /* Block special file? */
	#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO) /* FIFO special file?  */

	/* Mode bits. */
    #ifndef __APPLE__
        #define S_IRWXU  0700 /* Read, write, execute/search by owner.     */
        #define S_IRUSR  0400 /* Read permission, owner.                   */
        #define S_IWUSR  0200 /* Write permission, owner.                  */
        #define S_IXUSR  0100 /* Execute/search permission, owner.         */
        #define S_IRWXG   070 /* Read, write, execute/search by group.     */
        #define S_IRGRP   040 /* Read permission, group.                   */
        #define S_IWGRP   020 /* Write permission, group.                  */
        #define S_IXGRP   010 /* Execute/search permission, group.         */
        #define S_IRWXO    07 /* Read, write, execute/search by others.    */
        #define S_IROTH    04 /* Read permission, others.                  */
        #define S_IWOTH    02 /* Write permission, others.                 */
        #define S_IXOTH    01 /* Execute/search permission, others.        */
        #define S_ISUID 04000 /* Set-user-ID on execution.                 */
        #define S_ISGID 02000 /* Set-group-ID on execution.                */
        #define S_ISVTX 01000 /* On directories, restricted deletion flag. */
    #endif /* __APPLE__ */

	typedef long blkcnt_t;
	typedef long blksize_t;

#else

#include <sys/stat.h>

#endif /* !__unix64__*/

	typedef mode_t nanvix_mode_t;
	typedef uid_t nanvix_uid_t;
	typedef gid_t nanvix_gid_t;
	typedef blkcnt_t nanvix_blkcnt_t;
	typedef blksize_t nanvix_blksize_t;
	typedef dev_t nanvix_dev_t;
	typedef ino_t nanvix_ino_t;
	typedef int nanvix_nlink_t;
	typedef off_t nanvix_off_t;

	struct nanvix_stat
	{
		nanvix_dev_t st_dev;
		nanvix_ino_t st_ino;
		nanvix_mode_t st_mode;
		nanvix_nlink_t st_nlink;
		nanvix_uid_t st_uid;
		nanvix_gid_t st_gid;
		nanvix_dev_t st_rdev;
		nanvix_off_t st_size;
		struct nanvix_timespec *st_atim;
		struct nanvix_timespec *st_mtim;
		struct nanvix_timespec *st_ctim;
		nanvix_blksize_t st_blksize;
		nanvix_blkcnt_t st_blocks;
	};

#endif /* POSIX_SYS_STAT_H_ */
