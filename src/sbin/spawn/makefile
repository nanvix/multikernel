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

#===============================================================================
# Toolchain Configuration
#===============================================================================

# Modules Libraries
export MODULES := $(LIBDIR)/nanvix-hello.$(OBJ_SUFFIX).a \
                  $(LIBDIR)/nanvix-rmem.$(OBJ_SUFFIX).a  \
                  $(LIBDIR)/nanvix-shm.$(OBJ_SUFFIX).a   \
                  $(LIBDIR)/nanvix-name.$(OBJ_SUFFIX).a  \
                  $(LIBDIR)/nanvix-vfs.$(OBJ_SUFFIX).a

# C Source Files
export SRC = $(wildcard $(CURDIR)/*.c)

#===============================================================================

# Builds All Object Files
all:
	cd $(TARGET) && $(MAKE) all-spawn

# Cleans All Object Files
clean:
	cd $(TARGET) && $(MAKE) clean-spawn

# Cleans Everything
distclean: clean
	cd $(TARGET) && $(MAKE) distclean-spawn
