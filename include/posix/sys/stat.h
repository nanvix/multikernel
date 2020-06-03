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

#ifndef __unix64__

#ifndef POSIX_SYS_STAT_H_
#define POSIX_SYS_STAT_H_

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

#endif /* POSIX_SYS_STAT_H_ */

#else

#include <sys/stat.h>

#endif /* !__unix64__*/
