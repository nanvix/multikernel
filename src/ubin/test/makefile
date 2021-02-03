#
# MIT License
#
# Copyright(c) 2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

include $(BUILDDIR)/makefile.config

#===============================================================================
# Binaries Sources and Objects
#===============================================================================

# Binary
EXEC = nanvix-tests.$(OBJ_SUFFIX)

# C Source Files
SRC = $(wildcard *.c)              \
      $(wildcard name/*.c)         \
      $(wildcard fs/*.c)           \
      $(wildcard posix/*.c)        \
      $(wildcard proc/*.c)         \
      $(wildcard sysv/shm/*.c)     \
      $(wildcard sysv/*.c)         \
      $(wildcard rmem/manager/*.c) \
      $(wildcard rmem/cache/*.c)   \
      $(wildcard rmem/stub/*.c)

# Object Files
OBJ = $(SRC:.c=.$(OBJ_SUFFIX).o)

#===============================================================================

include $(BUILDDIR)/makefile.executable
